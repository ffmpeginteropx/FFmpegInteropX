#pragma once
#include "pch.h"
#include "SubtitleProvider.h"
#include "AttachedFileHelper.h"
#include "ass/ass.h"
#include "NativeBufferFactory.h"
#include <winrt/Windows.Graphics.Imaging.h>
#include <immintrin.h>
#include <winrt/Windows.Graphics.DirectX.Direct3D11.h>
#include "DirectXInteropHelper.h"
#include <ppl.h>

using namespace winrt::Windows::Storage::FileProperties;
using namespace winrt::Windows::Media::Core;
using namespace winrt::Windows::Foundation::Metadata;
using namespace winrt::Windows::Graphics::Imaging;

const float MIN_UINT8_CAST = 0.9F / 255;
const float MAX_UINT8_CAST = 255.9F / 255;
#define CLAMP_UINT8(value) ((value > MIN_UINT8_CAST) ? ((value < MAX_UINT8_CAST) ? (BYTE)(value * 255) : 255) : 0)

#define CLAMP_BYTE(value) ((value > 0) ? ((value < 255) ? (BYTE)value : (BYTE)255) : (BYTE)0)

template<typename Mutex>
struct anti_lock
{
    anti_lock() = default;

    explicit anti_lock(Mutex& mutex)
        : m_mutex(std::addressof(mutex)) {
        if (m_mutex) m_mutex->unlock();
    }

private:
    struct anti_lock_deleter {
        void operator()(Mutex* mutex) { mutex->lock(); }
    };

    std::unique_ptr<Mutex, anti_lock_deleter> m_mutex;
};

class SubtitleProviderLibass : public SubtitleProvider
{
public:
    SubtitleProviderLibass(std::shared_ptr<FFmpegReader> reader,
        AVFormatContext* avFormatCtx,
        AVCodecContext* avCodecCtx,
        MediaSourceConfig const& config,
        int index,
        std::shared_ptr<AttachedFileHelper> attachedFileHelper)
        : SubtitleProvider(reader,
            avFormatCtx,
            avCodecCtx,
            config,
            index,
            TimedMetadataKind::ImageSubtitle
        )
    {
        this->attachedFileHelper = attachedFileHelper;
    }

    virtual bool CanRenderSubtitles() override
    {
        return true;
    }

    virtual HRESULT SetHardwareDevice(winrt::com_ptr<ID3D11Device> newDevice,
        winrt::com_ptr<ID3D11DeviceContext> newDeviceContext,
        AVBufferRef* avHardwareContext) override
    {
        device = newDevice;
        deviceContext = newDeviceContext;
        return S_OK;
    };

    void ParseHeaders()
    {
        if (!hasParsedHeaders)
        {
            hasParsedHeaders = true;
            auto str = std::string((char*)m_pAvCodecCtx->subtitle_header, m_pAvCodecCtx->subtitle_header_size);

            InitializeLibass();

            track = ass_new_track(assLibrary);
            // why checking this?
            // embedded subtitle doesn't have Dialogue: tag since it has chunk
            // extrnal subtitles will load all ass sub using ass_read_memory
            // pass the header to ass to libass
            ass_process_codec_private(track, (char*)m_pAvCodecCtx->subtitle_header, m_pAvCodecCtx->subtitle_header_size);
        }
    }

    virtual void NotifyVideoFrameSize(int frameWidth, int frameHeight, double aspectRatio) override
    {
        videoAspectRatio = aspectRatio;
        videoWidth = frameWidth;
        videoHeight = frameHeight;
    }

    virtual winrt::com_ptr<implementation::SubtitleRenderResult>  RenderSubtitlesToDirectXSurface(winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DSurface rendertarget, TimeSpan position, bool forceRender) override
    {
        std::lock_guard lock(mutex);

        if (!assRenderer)
            return winrt::make_self<implementation::SubtitleRenderResult>();

        winrt::com_ptr<IDXGISurface> renderTargetDXGI;
        DirectXInteropHelper::GetDXGISurface(rendertarget, renderTargetDXGI);

        DXGI_SURFACE_DESC desc;
        renderTargetDXGI->GetDesc(&desc);

        if (desc.Format != DXGI_FORMAT_B8G8R8A8_UNORM)
        {
            OutputDebugString(L"Render surface for subtitles is not BGRA8!\n");
            return winrt::make_self<implementation::SubtitleRenderResult>();
        }
        if (desc.Width == 0 || desc.Height == 0)
        {
            OutputDebugString(L"Invalid surface size!\n");
            return winrt::make_self<implementation::SubtitleRenderResult>();
        }

        SetSubtitleSize(desc.Width, desc.Height);
        auto start = CalculatePosition(&position);
        int changes = 0;
        
        auto assImage = ass_render_frame(assRenderer, track, start, &changes);
        if (!changes && !forceRender)
        {
            //if no changes and the caller didn't request a force render, then nothing left to do.
            OutputDebugString(L"Subtitle frame is same as previous!\n");
            return winrt::make_self<implementation::SubtitleRenderResult>(true, false);
        }

        // get d3d interfaces
        winrt::com_ptr<ID3D11Resource> resource;
        winrt::com_ptr<ID3D11RenderTargetView> renderTargetView;
        winrt::com_ptr<ID3D11Device> device;
        winrt::com_ptr<ID3D11DeviceContext> deviceContext;
        resource = renderTargetDXGI.as<ID3D11Resource>();
        resource->GetDevice(device.put());
        device->GetImmediateContext(deviceContext.put());

        // Create render target view
        auto hr = device->CreateRenderTargetView(resource.get(), NULL, renderTargetView.put());
        if (!SUCCEEDED(hr)) return winrt::make_self<implementation::SubtitleRenderResult>();

        // Clear texture with transparent color
        const FLOAT transparent[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        deviceContext->ClearRenderTargetView(renderTargetView.get(), transparent);

        // If no image is provided, we are done here (empty scene) - this is not an error
        if (!assImage) {
            return winrt::make_self<implementation::SubtitleRenderResult>(true, changes != 0);
        }

        // blend ass_image to buffer
        auto pixelData = buffer.data();
        Blend(assImage, pixelData, subtitleWidth, subtitleHeight);
        auto rects = GetContentRects(assImage);
        auto width = subtitleWidth;

        {
            // use anti_lock to unlock and allow next ass_render and blend in parallel
            auto anti_guard = anti_lock(mutex);

            // upload to GPU
            UploadToTexture(pixelData, width, rects, deviceContext, resource);
        }

        return winrt::make_self<implementation::SubtitleRenderResult>(true, changes != 0);
    }

    virtual IMediaCue CreateCue(AVPacket* packet, winrt::Windows::Foundation::TimeSpan* position, winrt::Windows::Foundation::TimeSpan* duration) override
    {
        if (!IsEnabled()) return nullptr;

        std::lock_guard lock(mutex);
        ParseHeaders();
        AVSubtitle subtitle;

        int gotSubtitle = 0;
        auto result = avcodec_decode_subtitle2(m_pAvCodecCtx, &subtitle, &gotSubtitle, packet);
        if (result > 0 && gotSubtitle && subtitle.num_rects > 0)
        {
            auto ass = subtitle.rects[0]->ass;
            auto str = StringUtils::Utf8ToWString(ass);

            int64_t pos = CalculatePosition(position);
            int64_t dur = CalculatePosition(duration);

            auto data = (char*)ass;
            auto length = strlen(ass);

            // pass the subtitle chunk to libass
            ass_process_chunk(track, data, length, pos, dur);

            return nullptr;
        }
        return nullptr;
    }

    void Blend(ass_image* assImage, BYTE* pixelData, int width, int height)
    {
        // Clear out pixel data
        memset(pixelData, 0, width * height * 4);

        // TODO check if performance is better by doing blocks of lines vs alternating lines
        Concurrency::parallel_for(0, (int)threads, [&](int rowNum) {
            // Iterate through the ASS_Image linked list
            // could be replaced by shaders, directx math or vector intrinsics
            for (ASS_Image* img = assImage; img != nullptr; img = img->next)
            {
                /*
                TODO: we actually need a test case for this.
                if (!img->w || !img->h) continue; // we shouldn't render this image according to docs*/

                uint8_t* src = img->bitmap;
                int stride = img->stride;
                uint32_t color = img->color;

                uint8_t a = 255 - color & 0xFF;
                uint8_t b = (color >> 8) & 0xFF;
                uint8_t g = (color >> 16) & 0xFF;
                uint8_t r = (color >> 24) & 0xFF;

                float normalizedAlpha = a / 255.0f;

                // If alpha is 0, skip blending
                if (a == 0) continue;

                int startRow = rowNum - (img->dst_y % threads);
                if (startRow < img->dst_y)
                {
                    startRow += threads;
                }

                for (int y = startRow; y < img->h; y += threads) {
                    for (int x = 0; x < img->w; ++x)
                    {
                        int srcIndex = y * stride + x;
                        int destIndex = ((img->dst_y + y) * width + (img->dst_x + x)) * 4;

                        float srcAlpha = (src[srcIndex] * normalizedAlpha) / 255.0f; // Scale alpha by the bitmap's alpha channel
                        float invertAlpha = 1.0f - srcAlpha;

                        // If alpha is 0, skip blending
                        if (srcAlpha == 0) continue;

                        pixelData[destIndex + 0] = CLAMP_BYTE(pixelData[destIndex + 0] * invertAlpha + b * srcAlpha);       // Blue
                        pixelData[destIndex + 1] = CLAMP_BYTE(pixelData[destIndex + 1] * invertAlpha + g * srcAlpha);       // Green
                        pixelData[destIndex + 2] = CLAMP_BYTE(pixelData[destIndex + 2] * invertAlpha + r * srcAlpha);       // Red
                        pixelData[destIndex + 3] = CLAMP_BYTE(pixelData[destIndex + 3] * invertAlpha + 255 * srcAlpha);     // Alpha
                    }
                }
            }
            });
    }

    void UploadToTexture(BYTE* pixelData, int linesize, std::vector<D3D11_RECT> rects,
        winrt::com_ptr<ID3D11DeviceContext> deviceContext, winrt::com_ptr<ID3D11Resource> resource)
    {
        // Paint rects
        for (auto& rect : rects)
        {
            D3D11_BOX box{
                rect.left,
                rect.top,
                0,
                rect.right,
                rect.bottom,
                1
            };
            auto dataPtr = pixelData + rect.left * 4 + rect.top * (linesize * 4);
            deviceContext->UpdateSubresource(resource.get(), 0, &box, dataPtr, linesize * 4, 0);
        }
    }

    std::vector<D3D11_RECT> GetContentRects(ASS_Image* assImage)
    {
        std::vector<D3D11_RECT> rects;
        auto img = assImage;
        while (img)
        {
            D3D11_RECT img_rect{
                img->dst_x,
                img->dst_y,
                img->dst_x + img->w,
                img->dst_y + img->h
            };
            bool found = false;
            for (int i = 0; i < rects.size(); i++)
            {
                auto& rect = rects[i];
                if (Overlaps(rect, img_rect))
                {
                    rects[i] = Merge(rect, img_rect);
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                rects.push_back(img_rect);
            }
            img = img->next;
        }

        return rects;
    }

    D3D11_RECT Merge(D3D11_RECT rect, D3D11_RECT other)
    {
        return {
            min(rect.left, other.left),
            min(rect.top, other.top),
            max(rect.right, other.right),
            max(rect.bottom, other.bottom)
        };
    }

    bool Overlaps(const D3D11_RECT& rect1, const D3D11_RECT& rect2) {
        return !(rect1.right <= rect2.left || rect1.left >= rect2.right ||
            rect1.bottom <= rect2.top || rect1.top >= rect2.bottom);
    }

    bool Contains(D3D11_RECT rect, int x, int y)
    {
        return x >= rect.left && x <= rect.right && y >= rect.top && y <= rect.bottom;
    }

    void InitializeLibass()
    {
        assLibrary = ass_library_init();
        if (!assLibrary)
        {
            OutputDebugString(L"ass_library_init failed!\r\n");
            return;
        }
        ass_set_message_cb(assLibrary, messageCallback, NULL);

        // fixes fonts issues
        ass_set_extract_fonts(assLibrary, true);

        assRenderer = ass_renderer_init(assLibrary);
        if (!assRenderer)
        {
            OutputDebugString(L"ass_renderer_init failed!\r\n");
            return;
        }

        //send native video size, if available
        if (videoWidth > 0 && videoHeight > 0)
        {
            ass_set_storage_size(assRenderer, videoWidth, videoHeight);
        }

        SetSubtitleSize(1920, 1080);

        SetFonts();

        threads = std::thread::hardware_concurrency();
        threads /= 4; // don't go too far, overhead increases a lot
        if (threads < 2)
        {
            threads = 2; // at least 2 threads
        }
    }

    void SetSubtitleSize(int width, int height)
    {
        if (width != subtitleWidth || height != subtitleHeight)
        {
            subtitleWidth = width;
            subtitleHeight = height;

            if (assRenderer)
                ass_set_frame_size(assRenderer, width, height);

            auto size = width * height * 4;
            buffer = NativeBuffer::NativeBufferFactory::CreateNativeBuffer(size);
        }
    }

    void SetFonts()
    {
        try {
            if (ExtractFonts())
            {
                auto fontFolder = attachedFileHelper->GetInstanceFolder().get();
                auto fontDirectory = StringUtils::PlatformStringToUtf8String(fontFolder.Path());
                ass_set_fonts_dir(assLibrary, fontDirectory.data());
            }
            ass_set_fonts(assRenderer, NULL, "Segoe UI", ASS_FONTPROVIDER_AUTODETECT, nullptr, 0);
        }
        catch (exception e)
        {
            auto x = e.what();
        }
    }

    bool ExtractFonts()
    {
        bool hasFonts = false;
        try
        {
            if (m_config.Subtitles().UseEmbeddedSubtitleFonts())
            {
                for (auto& attachment : attachedFileHelper->AttachedFiles())
                {
                    std::wstring mime(attachment->MimeType());
                    if (mime.find(L"font") != mime.npos)
                    {
                        attachedFileHelper->ExtractFileAsync(attachment).get();
                        hasFonts = true;
                    }
                }
            }
        }
        catch (...)
        {
        }
        return hasFonts;
    }

    void FreeLibass()
    {
        if (track)
            ass_free_track(track);

        if (assRenderer)
            ass_renderer_done(assRenderer);

        if (assLibrary)
            ass_library_done(assLibrary);
    }

    SoftwareBitmap GetDummyBitmap()
    {
        if (!dummyBitmap)
        {
            dummyBitmap = SoftwareBitmap(BitmapPixelFormat::Bgra8, 16, 16, BitmapAlphaMode::Premultiplied);
        }

        return dummyBitmap;
    }

    static int64_t CalculatePosition(TimeSpan* time)
    {
        return time->count() / 10'000;
    }

    static void messageCallback(int level, const char* fmt, va_list va, void* data)
    {
        // only log info and higher
        if (level <= 5)
        {
            OutputDebugString(L"libass: ");
            char buffer[1024];
            vsnprintf(buffer, sizeof(buffer), fmt, va);

            std::wstring wideMessage = StringUtils::ConvertStringToWString(buffer);
            OutputDebugString(wideMessage.c_str());

            OutputDebugString(L"\r\n");
        }
    }

    ~SubtitleProviderLibass()
    {
        FreeLibass();
    }

private:
    bool hasParsedHeaders = false;
    double videoAspectRatio = 0.0;
    int videoWidth = 0;
    int videoHeight = 0;
    int subtitleWidth = 1920;
    int subtitleHeight = 1080;
    ASS_Library* assLibrary = nullptr;
    ASS_Renderer* assRenderer = nullptr;
    ASS_Track* track = nullptr;
    IBuffer buffer = { nullptr };
    int logLevel = 3;
    int minX = 0;
    int minY = 0;
    SoftwareBitmap dummyBitmap = { nullptr };
    int nextId = 0;
    unsigned threads = 4;

    std::shared_ptr<AttachedFileHelper> attachedFileHelper;
};
