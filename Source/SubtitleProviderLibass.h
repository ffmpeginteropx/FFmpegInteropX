#pragma once
#include "pch.h"
#include "SubtitleProvider.h"
#include "AttachedFileHelper.h"
#include "ass/ass.h"
#include "NativeBufferFactory.h"
#include <winrt/Windows.Graphics.Imaging.h>

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
#define CLAMP_UINT8(value) ((value > MIN_UINT8_CAST) ? ((value < MAX_UINT8_CAST) ? (int)(value * 255) : 255) : 0)


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
        DispatcherQueue const& dispatcher)
        : SubtitleProvider(reader,
            avFormatCtx,
            avCodecCtx,
            config,
            index,
            TimedMetadataKind::ImageSubtitle,
            dispatcher
        )
    {
    }

    virtual HRESULT Initialize() override
    {
        auto hr = SubtitleProvider::Initialize();

        if (SUCCEEDED(hr))
        {
            SubtitleTrack.CueEntered(weak_handler(this, &SubtitleProviderLibass::OnCueEntered));
            SubtitleTrack.CueExited(weak_handler(this, &SubtitleProviderLibass::OnCueExited));
        }

        return S_OK;
    }
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

    RenderBlendResult* SubtitleProviderLibass::Blend(long long time, int force)
    {
        m_blendResult.blend_time = 0.0;
        m_blendResult.image = NULL;

        ASS_Image* img = ass_render_frame(assRenderer, track, time, &m_blendResult.changed);
        if (img == NULL || (m_blendResult.changed == 0 && !force))
        {
            return &m_blendResult;
        }

        int min_x = img->dst_x, min_y = img->dst_y;
        int max_x = img->dst_x + img->w - 1, max_y = img->dst_y + img->h - 1;
        for (auto cur = img->next; cur != NULL; cur = cur->next)
        {
            if (cur->dst_x < min_x) min_x = cur->dst_x;
            if (cur->dst_y < min_y) min_y = cur->dst_y;
            int right = cur->dst_x + cur->w - 1;
            int bottom = cur->dst_y + cur->h - 1;
            if (right > max_x) max_x = right;
            if (bottom > max_y) max_y = bottom;
        }

        // copied from Subtitle Octapus
        int width = max_x - min_x + 1, height = max_y - min_y + 1;
        auto size = sizeof(float) * width * height * 4;
        float* buf = (float*)buffer_resize(&m_blend, size, 0);

        if (buf == NULL)
        {
            OutputDebugString(L"libass: error: cannot allocate buffer for blending");
            return &m_blendResult;
        }

        memset(buf, 0, sizeof(float) * width * height * 4);

        for (auto cur = img; cur != NULL; cur = cur->next)
        {
            int curw = cur->w, curh = cur->h;
            if (curw == 0 || curh == 0) continue; // skip empty images
            int a = (255 - (cur->color & 0xFF));
            if (a == 0) continue; // skip transparent images

            int curs = (cur->stride >= curw) ? cur->stride : curw;
            int curx = cur->dst_x - min_x, cury = cur->dst_y - min_y;

            unsigned char* bitmap = cur->bitmap;
            float normalized_a = a / 255.0f;
            float r = ((cur->color >> 24) & 0xFF) / 255.0f;
            float g = ((cur->color >> 16) & 0xFF) / 255.0f;
            float b = ((cur->color >> 8) & 0xFF) / 255.0f;

            int buf_line_coord = cury * videoWidth;
            for (int y = 0, bitmap_offset = 0; y < curh; y++, bitmap_offset += curs, buf_line_coord += videoWidth)
            {
                for (int x = 0; x < curw; x++)
                {
                    float pix_alpha = bitmap[bitmap_offset + x] * normalized_a / 255.0f;
                    float inv_alpha = 1.0 - pix_alpha;

                    int buf_coord = (buf_line_coord + curx + x) << 2;
                    float* buf_r = buf + buf_coord;
                    float* buf_g = buf + buf_coord + 1;
                    float* buf_b = buf + buf_coord + 2;
                    float* buf_a = buf + buf_coord + 3;

                    // do the compositing, pre-multiply image RGB with alpha for current pixel
                    *buf_a = pix_alpha + *buf_a * inv_alpha;
                    *buf_r = r * pix_alpha + *buf_r * inv_alpha;
                    *buf_g = g * pix_alpha + *buf_g * inv_alpha;
                    *buf_b = b * pix_alpha + *buf_b * inv_alpha;
                }
            }
        }

        // now build the result;
        // NOTE: we use a "view" over [float,float,float,float] array of pixels,
        // so we _must_ go left-right top-bottom to not mangle the result
        unsigned int* result = (unsigned int*)buf;
        for (int y = 0, buf_line_coord = 0; y < height; y++, buf_line_coord += width)
        {
            for (int x = 0; x < width; x++)
            {
                unsigned int pixel = 0;
                int buf_coord = (buf_line_coord + x) << 2;
                float alpha = buf[buf_coord + 3];
                if (alpha > MIN_UINT8_CAST)
                {
                    // need to un-multiply the result
                    float value = buf[buf_coord] / alpha;
                    pixel |= CLAMP_UINT8(value); // R
                    value = buf[buf_coord + 1] / alpha;
                    pixel |= CLAMP_UINT8(value) << 8; // G
                    value = buf[buf_coord + 2] / alpha;
                    pixel |= CLAMP_UINT8(value) << 16; // B
                    pixel |= CLAMP_UINT8(alpha) << 24; // A
                }
                result[buf_line_coord + x] = pixel;
            }
        }

        m_blendResult.dest_x = min_x;
        m_blendResult.dest_y = min_y;
        m_blendResult.dest_width = width;
        m_blendResult.dest_height = height;
        m_blendResult.image = (unsigned char*)result;
        m_blendResult.size = size;
        return &m_blendResult;
    }


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
        if (width <= 0)
            width = 1920;
        if (height <= 0)
            height = 1080;
        if (!assImage) {
            throw std::invalid_argument("ASS_Image is null");
        }

        BitmapPixelFormat pixelFormat = BitmapPixelFormat::Bgra8;
        BitmapAlphaMode alphaMode = BitmapAlphaMode::Premultiplied;
        SoftwareBitmap bitmap = SoftwareBitmap(pixelFormat, width, height, alphaMode);
        
        auto buffer = bitmap.LockBuffer(BitmapBufferAccessMode::ReadWrite);
        // Create a buffer to hold the final image (BGRA format)
        auto reference = buffer.CreateReference();
        auto pixelData = reference.data();

        // Iterate through the ASS_Image linked list
        for (ASS_Image* img = assImage; img != nullptr; img = img->next)
        {
            uint8_t* src = img->bitmap;
            int stride = img->stride;
            uint32_t color = img->color;

            //R = (byte)(color & 0xff);
            //G = (byte)((color >> 8) & 0xff);
            //B = (byte)((color >> 16) & 0xff);
            //A = (byte)((color >> 24) & 0xff);
            uint8_t a = (color >> 24) & 0xFF;
            uint8_t r = (color >> 16) & 0xFF;
            uint8_t g = (color >> 8) & 0xFF;
            uint8_t b = color & 0xFF;

            // Process each pixel in the current ASS_Image
            for (int y = 0; y < img->h; ++y)
            {
                for (int x = 0; x < img->w; ++x)
                {
                    int srcIndex = y * stride + x;
                    int destIndex = ((img->dst_y + y) * width + (img->dst_x + x)) * 4;

                    uint8_t srcAlpha = (src[srcIndex] * a) / 255; // Scale alpha by the bitmap's alpha channel

                    // If alpha is 0, skip blending
                    if (srcAlpha == 0) continue;

                    pixelData[destIndex + 0] = b;       // Blue
                    pixelData[destIndex + 1] = g;       // Green
                    pixelData[destIndex + 2] = r;       // Red
                    pixelData[destIndex + 3] = srcAlpha; // Alpha
                }
            }
        }
       
        return bitmap;
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

        /* if (width > 0 && height > 0)*/
        ass_set_frame_size(assRenderer, width, height);
    }

    void SetFonts()
    { 
        ass_set_fonts(assRenderer, NULL, "Segoe UI", ASS_FONTPROVIDER_AUTODETECT, nullptr, 0);
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

    int64_t CalculatePosition(TimeSpan *time)
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
};
