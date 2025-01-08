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
#include <LibassBlenderPixelShaderBlob.h>

using namespace winrt::Windows::Storage::FileProperties;
using namespace winrt::Windows::Media::Core;
using namespace winrt::Windows::Foundation::Metadata;
using namespace winrt::Windows::Graphics::Imaging;

#ifdef Win32
using namespace winrt::Microsoft::UI::Dispatching;
#else
using namespace winrt::Windows::System;
#endif

const float MIN_UINT8_CAST = 0.9F / 255;
const float MAX_UINT8_CAST = 255.9F / 255;
#define CLAMP_UINT8(value) ((value > MIN_UINT8_CAST) ? ((value < MAX_UINT8_CAST) ? (BYTE)(value * 255) : 255) : 0)

#define CLAMP_BYTE(value) ((value > 0) ? ((value < 255) ? (BYTE)value : (BYTE)255) : (BYTE)0)


typedef struct
{
public:
    int changed;
    double blend_time;
    int dest_x, dest_y, dest_width, dest_height;
    unsigned char* image;
    int size;
} RenderBlendResult;

typedef struct {
    void* buffer;
    int size;
    int lessen_counter;
} buffer_t;


class SubtitleProviderLibass : public SubtitleProvider
{
public:
    SubtitleProviderLibass(std::shared_ptr<FFmpegReader> reader,
        AVFormatContext* avFormatCtx,
        AVCodecContext* avCodecCtx,
        MediaSourceConfig const& config,
        int index,
        DispatcherQueue const& dispatcher,
        std::shared_ptr<AttachedFileHelper> attachedFileHelper)
        : SubtitleProvider(reader,
            avFormatCtx,
            avCodecCtx,
            config,
            index,
            TimedMetadataKind::ImageSubtitle,
            dispatcher
        )
    {
        this->attachedFileHelper = attachedFileHelper;
    }

    virtual HRESULT Initialize() override
    {
        auto hr = SubtitleProvider::Initialize();

        if (SUCCEEDED(hr))
        {
            //SubtitleTrack.CueEntered(weak_handler(this, &SubtitleProviderLibass::OnCueEntered));
            //SubtitleTrack.CueExited(weak_handler(this, &SubtitleProviderLibass::OnCueExited));
        }

        return S_OK;
    }

    virtual HRESULT SetHardwareDevice(winrt::com_ptr<ID3D11Device> newDevice,
        winrt::com_ptr<ID3D11DeviceContext> newDeviceContext,
        AVBufferRef* avHardwareContext) override
    {
        device = newDevice;
        deviceContext = newDeviceContext;
        return S_OK;
    };

    void SubtitleProviderLibass::SetPosition(winrt::Windows::Foundation::TimeSpan  position)
    {
        currentPosition = position;
    }

    void ParseHeaders()
    {
        if (!hasParsedHeaders)
        {
            hasParsedHeaders = true;
            auto str = std::string((char*)m_pAvCodecCtx->subtitle_header, m_pAvCodecCtx->subtitle_header_size);

            InitializeLibass();

            // i don't know how to get encoding page, to I just pass NULL
            //track = ass_read_memory(assLibrary, (char*)m_pAvCodecCtx->subtitle_header, m_pAvCodecCtx->subtitle_header_size, NULL);
            track = ass_new_track(assLibrary);
            // why checking this?
            // embedded subtitle doesn't have Dialogue: tag since it has chunk
            // extrnal subtitles will load all ass sub using ass_read_memory
            //auto isEventsAvailable = str.find("Dialogue:");
            //if (isEventsAvailable == str.npos)
            {
                // pass the header to ass to libass
                ass_process_codec_private(track, (char*)m_pAvCodecCtx->subtitle_header, m_pAvCodecCtx->subtitle_header_size);
            }
        }
    }

    virtual void NotifyVideoFrameSize(int frameWidth, int frameHeight, double aspectRatio) override
    {
        videoAspectRatio = aspectRatio;
        videoWidth = frameWidth;
        videoHeight = frameHeight;
    }

    virtual SoftwareBitmap RenderSubtitles(winrt::Windows::Foundation::TimeSpan videoPosition, Size const& renderSize) override
    {
        std::lock_guard lock(mutex);
        if (!assRenderer)
            return nullptr;

        SetSubtitleSize(renderSize.Width, renderSize.Height);
        auto start = CalculatePosition(&videoPosition);
        auto image = ass_render_frame(assRenderer, track, start, 0);
        auto bitmap = ConvertASSImageToSoftwareBitmap(image, subtitleWidth, subtitleHeight);
        return bitmap;
    }

    virtual bool RenderSubtitlesToDirectXSurface(winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DSurface rendertarget, TimeSpan videoPosition, Size const& renderSize) override
    {
        std::lock_guard lock(mutex);
        if (!assRenderer)
            return false;

        SetSubtitleSize(renderSize.Width, renderSize.Height);
        auto start = CalculatePosition(&videoPosition);
        auto image = ass_render_frame(assRenderer, track, start, 0);
        return ConvertASSImageToSoftwareBitmapDirectXSurface(rendertarget, image, subtitleWidth, subtitleHeight);
    }

    virtual IMediaCue CreateCue(AVPacket* packet, winrt::Windows::Foundation::TimeSpan* position, winrt::Windows::Foundation::TimeSpan* duration) override
    {
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

            wchar_t buffer[256];
            swprintf_s(buffer, L">>>>>>>Added Pos %02d      dur: %02d\n",
                pos, dur);

            OutputDebugString(buffer);

            auto data = (char*)ass;
            auto length = strlen(ass);

            // pass the subtitle chunk to libass
            ass_process_chunk(track, data, length, pos, dur);

            auto id = winrt::to_hstring(nextId++);
            ImageCue cue;
            cue.Id(id);

            // rendering subtitle in here cause performance issues a lot
            //int changes = 0;
            //auto image = ass_render_frame(assRenderer, track, cur, &changes);
            //CreateSubtitleImage(cue, image);

            return cue;
        }
        return nullptr;
    }

    // this will be removed!

    void OnCueEntered(TimedMetadataTrack sender, MediaCueEventArgs args)
    {
        std::lock_guard lock(mutex);
        try
        {
            auto cue = args.Cue().try_as<ImageCue>();
            if (cue)
            {
                int changes = 1;
                auto cur = CalculatePosition(&currentPosition);
                auto start = CalculatePosition(&cue.StartTime());
                auto duration = CalculatePosition(&cue.Duration());

                // libass: Event at 19002, +294: 53,0,Main,Glasses,0,0,0,,...وانیکا ساما، خواهش میکنم
                // Start rendering frame at 190020, duration: 2940
                // Failed to render frame.
                // libass: Event at 19296, +325: 54,0,Main,Halbert,0,0,0,,—صبرکنید، وانیــ
                // Start rendering frame at 192960, duration: 3250
                // Failed to render frame.

                wchar_t buffer[256];
                swprintf_s(buffer, L"Start rendering frame at %02d, start:%02d  duration: %02d\r\n",
                    cur, start, duration);

                OutputDebugString(buffer);

                auto image = ass_render_frame(assRenderer, track, start, 0);
                CreateSubtitleImage(cue, image);
            }
        }
        catch (...)
        {
            OutputDebugString(L"Failed to render image cue.");
        }
    }

    void OnCueExited(TimedMetadataTrack sender, MediaCueEventArgs args)
    {
        std::lock_guard lock(mutex);
        try
        {
            auto cue = args.Cue().try_as<ImageCue>();
            if (cue)
            {
                cue.SoftwareBitmap(GetDummyBitmap());
            }
        }
        catch (...)
        {
            OutputDebugString(L"Failed to clear image cue.");
        }
    }

    void CreateSubtitleImage(ImageCue cue, ASS_Image* img)
    {
        if (img && cue)
        {
            OutputDebugString(L"Frame rendered.\r\n");
            TimedTextSize cueSize{};
            TimedTextPoint cuePosition{};

            cueSize.Unit = TimedTextUnit::Percentage;
            cueSize.Width = 100;
            cueSize.Height = 100;

            cuePosition.Unit = TimedTextUnit::Percentage;
            cuePosition.X = 0;
            cuePosition.Y = 0;

            auto bitmap = ConvertASSImageToSoftwareBitmap(img, subtitleWidth, subtitleHeight);
            cue.SoftwareBitmap(bitmap);
            cue.Position(cuePosition);
            cue.Extent(cueSize);

            OutputDebugString(L"Frame added to cue.\r\n");
        }
        else
        {
            OutputDebugString(L"Failed to render frame.\r\n");
            cue.SoftwareBitmap(GetDummyBitmap());
        }
    }

    SoftwareBitmap ConvertASSImageToSoftwareBitmap(ASS_Image* assImage, int width, int height)
    {
        if (width <= 0 || height <= 0)
            return nullptr;
        if (!assImage) {
            //throw std::invalid_argument("ASS_Image is null");
            OutputDebugString(L"ASS_Image is null\n");
            return nullptr;
        }

        // Create a buffer to hold the final image (BGRA format)
        auto size = width * height * 4;
        auto buffer = winrt::Windows::Storage::Streams::Buffer(static_cast<uint32_t>(size));
        auto pixelData = buffer.data();
        memset(pixelData, 0, size);// Initialize with zeros for transparent background
        buffer.Length(static_cast<uint32_t>(size));

        // Iterate through the ASS_Image linked list
        for (ASS_Image* img = assImage; img != nullptr; img = img->next)
        {
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

            // Process each pixel in the current ASS_Image
            for (int y = 0; y < img->h; ++y)
            {
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

        BitmapPixelFormat pixelFormat = BitmapPixelFormat::Bgra8;
        BitmapAlphaMode alphaMode = BitmapAlphaMode::Premultiplied;
        SoftwareBitmap bitmap = SoftwareBitmap::CreateCopyFromBuffer(buffer, pixelFormat, width, height, alphaMode);
        return bitmap;
    }

    bool ConvertASSImageToSoftwareBitmapDirectXSurface(winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DSurface rendertarget, ASS_Image* assImage, int width, int height)
    {
        if (width <= 0 || height <= 0)
            return false;
        if (!assImage) {
            //throw std::invalid_argument("ASS_Image is null");
            OutputDebugString(L"ASS_Image is null\n");
            return false;
        }

        winrt::com_ptr<IDXGISurface> renderTargetDXGI;
        winrt::com_ptr<ID3D11Texture2D> renderTargetTexture;
        DirectXInteropHelper::GetDXGISurface(rendertarget, renderTargetDXGI);

        DXGI_SURFACE_DESC targetargetDXGIDesc;
        renderTargetDXGI->GetDesc(&targetargetDXGIDesc);

        renderTargetTexture = renderTargetDXGI.as<ID3D11Texture2D>();
        D3D11_TEXTURE2D_DESC desc;
        renderTargetTexture->GetDesc(&desc);

        winrt::com_ptr<ID3D11Device> targetTextureDevice;
        winrt::com_ptr<ID3D11DeviceContext> targetTextureDeviceContext;
        renderTargetTexture->GetDevice(targetTextureDevice.put());
        targetTextureDevice->GetImmediateContext(targetTextureDeviceContext.put());

        auto size = width * height * 4;
        auto buffer = NativeBuffer::NativeBufferFactory::CreateZerosNativeBuffer(size);
        auto pixelData = buffer.data();

        // Iterate through the ASS_Image linked list
        //should be replaced by shaders or directx math
        for (ASS_Image* img = assImage; img != nullptr; img = img->next)
        {
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

            // Process each pixel in the current ASS_Image
            for (int y = 0; y < img->h; ++y)
            {
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


        D3D11_SUBRESOURCE_DATA initData = {};
        initData.pSysMem = pixelData;
        initData.SysMemPitch = width * 4;
        initData.SysMemSlicePitch = 0;

        winrt::com_ptr<ID3D11Texture2D> stagingTexture;
        winrt::com_ptr<ID3D11ShaderResourceView> srv;       

        DirectXInteropHelper::CreateTextureFromByteArray(targetTextureDevice.get(), pixelData, size, width, height, DXGI_FORMAT_B8G8R8A8_UNORM, stagingTexture.put());
        targetTextureDeviceContext->CopyResource(renderTargetTexture.get(), stagingTexture.get());
        targetTextureDeviceContext->Flush();
        return true;
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

        //TODO add API for setting the subtitle target size
        SetSubtitleSize(1920, 1080);

        SetFonts();
    }

    void SetSubtitleSize(int width, int height)
    {
        subtitleWidth = width;
        subtitleHeight = height;

        if (assRenderer)
            ass_set_frame_size(assRenderer, width, height);
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

    int64_t CalculatePosition(TimeSpan* time)
    {
        return time->count() / 10'000;
    }

    static void messageCallback(int level, const char* fmt, va_list va, void* data)
    {
        OutputDebugString(L"libass: ");
        char buffer[1024];
        vsnprintf(buffer, sizeof(buffer), fmt, va);

        std::wstring wideMessage = StringUtils::ConvertStringToWString(buffer);
        OutputDebugString(wideMessage.c_str());

        OutputDebugString(L"\r\n");
    }

    void* buffer_resize(buffer_t* buf, int new_size, int keep_content)
    {
        if (buf->size >= new_size)
        {
            if (buf->size >= 1.3 * new_size)
            {
                // big reduction request
                buf->lessen_counter++;
            }
            else
            {
                buf->lessen_counter = 0;
            }
            if (buf->lessen_counter < 10)
            {
                // not reducing the buffer yet
                return buf->buffer;
            }
        }

        void* newbuf;
        if (keep_content)
        {
            newbuf = realloc(buf->buffer, new_size);
        }
        else
        {
            newbuf = malloc(new_size);
        }
        if (!newbuf) return NULL;

        if (!keep_content) free(buf->buffer);
        buf->buffer = newbuf;
        buf->size = new_size;
        buf->lessen_counter = 0;
        return buf->buffer;
    }

    void buffer_init(buffer_t* buf)
    {
        buf->buffer = NULL;
        buf->size = -1;
        buf->lessen_counter = 0;
    }

    void buffer_free(buffer_t* buf)
    {
        free(buf->buffer);
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
    int logLevel = 3;
    int minX = 0;
    int minY = 0;
    buffer_t m_blend;
    RenderBlendResult m_blendResult;
    SoftwareBitmap dummyBitmap = { nullptr };
    int nextId = 0;
    TimeSpan currentPosition{ 0 };

    std::shared_ptr<AttachedFileHelper> attachedFileHelper;
};
