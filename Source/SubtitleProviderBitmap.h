#pragma once

#include "SubtitleProvider.h"
#include <winrt/FFmpegInteropX.h>

namespace FFmpegInteropX
{
    using namespace winrt::Windows::Graphics::Imaging;
    using namespace winrt::Windows::Media::Core;

    struct __declspec(uuid("5b0d3235-4dba-4d44-865e-8f1d0e4fd04d")) __declspec(novtable) IMemoryBufferByteAccess : ::IUnknown
    {
        virtual HRESULT __stdcall GetBuffer(uint8_t** value, uint32_t* capacity) = 0;
    };

    class SubtitleProviderBitmap : public SubtitleProvider
    {

    public:
        SubtitleProviderBitmap(std::shared_ptr<FFmpegReader> reader,
            AVFormatContext* avFormatCtx,
            AVCodecContext* avCodecCtx,
            winrt::FFmpegInteropX::MediaSourceConfig const& config,
            int index,
            winrt::Windows::UI::Core::CoreDispatcher  const& dispatcher)
            : SubtitleProvider(reader, avFormatCtx, avCodecCtx, config, index,TimedMetadataKind::ImageSubtitle, dispatcher)
        {
        }

        virtual void NotifyVideoFrameSize(int width, int height, double aspectRatio) override
        {
            videoWidth = width;
            videoHeight = height;
            if (isnormal(aspectRatio) && aspectRatio > 0)
            {
                videoAspectRatio = aspectRatio;
            }
            else
            {
                videoAspectRatio = (double)width / height;
            }
        }

        virtual IMediaCue CreateCue(AVPacket* packet, TimeSpan* position, TimeSpan* duration) override
        {
            // only decode image subtitles if the stream is selected
            if (!IsEnabled())
            {
                return nullptr;
            }

            AVSubtitle subtitle;
            int gotSubtitle = 0;
            auto result = avcodec_decode_subtitle2(m_pAvCodecCtx, &subtitle, &gotSubtitle, packet);
            if (result > 0 && gotSubtitle)
            {
                if (subtitle.start_display_time > 0)
                {
                    *position = TimeSpan(position->count() + (long long)10000 * subtitle.start_display_time);
                }
                *duration = TimeSpan((long long)10000 * subtitle.end_display_time);

                if (subtitle.num_rects <= 0)
                {
                    if (!dummyBitmap)
                    {
                        dummyBitmap = SoftwareBitmap(BitmapPixelFormat::Bgra8, 16, 16, BitmapAlphaMode::Premultiplied);
                    }

                    // inserty dummy cue
                   ImageCue cue =ImageCue();
                    cue.SoftwareBitmap(dummyBitmap);
                    avsubtitle_free(&subtitle);

                    return cue;
                }

                int width, height, offsetX, offsetY;
               TimedTextSize cueSize;
               TimedTextPoint cuePosition;
                if (subtitle.num_rects > 0 && CheckSize(subtitle, width, height, offsetX, offsetY, cueSize, cuePosition))
                {
                    auto bitmap = SoftwareBitmap(BitmapPixelFormat::Bgra8, width, height, BitmapAlphaMode::Straight);
                    {
                        auto buffer = bitmap.LockBuffer(BitmapBufferAccessMode::Write);
                        auto reference = buffer.CreateReference();

                        // Query the IBufferByteAccess interface.  
                        winrt::com_ptr<IMemoryBufferByteAccess> bufferByteAccess;
                        reference.as(IID_PPV_ARGS(&bufferByteAccess));

                        // Retrieve the buffer data.  
                        BYTE* pixels = nullptr;
                        unsigned int capacity;
                        bufferByteAccess->GetBuffer(&pixels, &capacity);

                        auto plane = buffer.GetPlaneDescription(0);

                        for (unsigned int i = 0; i < subtitle.num_rects; i++)
                        {
                            auto rect = subtitle.rects[i];

                            for (int y = 0; y < rect->h; y++)
                            {
                                for (int x = 0; x < rect->w; x++)
                                {
                                    auto inPointer = rect->data[0] + y * rect->linesize[0] + x;
                                    auto color = inPointer[0];
                                    if (color < rect->nb_colors)
                                    {
                                        auto rgba = ((uint32_t*)rect->data[1])[color];
                                        auto outPointer = pixels + plane.StartIndex + plane.Stride * ((y + rect->y) - offsetY) + 4 * ((x + rect->x) - offsetX);
                                        ((uint32_t*)outPointer)[0] = rgba;
                                    }
                                    else
                                    {
                                        OutputDebugString(L"Error: Illegal subtitle color.");
                                    }
                                }
                            }
                        }
                    }

                   ImageCue cue =ImageCue();
                    cue.SoftwareBitmap(SoftwareBitmap::Convert(bitmap, BitmapPixelFormat::Bgra8, BitmapAlphaMode::Premultiplied));
                    cue.Position(cuePosition);
                    cue.Extent(cueSize);

                    avsubtitle_free(&subtitle);

                    return cue;
                }
                else if (subtitle.num_rects > 0)
                {
                    OutputDebugString(L"Error: Invalid subtitle size received.");
                }

                avsubtitle_free(&subtitle);
            }
            else if (result <= 0)
            {
                OutputDebugString(L"Failed to decode subtitle.");
            }

            return nullptr;
        }

    private:

        bool CheckSize(AVSubtitle& subtitle, int& width, int& height, int& offsetX, int& offsetY,TimedTextSize& cueSize,TimedTextPoint& cuePosition)
        {
            if (!GetInitialSize())
            {
                return false;
            }

            // get actual extent of subtitle rects
            int minX = subtitleWidth, minY = subtitleHeight, maxW = 0, maxH = 0;
            for (unsigned int i = 0; i < subtitle.num_rects; i++)
            {
                auto rect = subtitle.rects[i];
                minX = min(minX, rect->x);
                minY = min(minY, rect->y);
                maxW = max(maxW, rect->x + rect->w);
                maxH = max(maxH, rect->y + rect->h);
            }

            // sanity check
            if (minX < 0 || minY < 0 || maxW > subtitleWidth || maxH > subtitleHeight)
            {
                return false;
            }

            offsetX = minX;
            offsetY = minY;
            width = maxW - minX;
            height = maxH - minY;

            // try to fit into actual video frame aspect ratio, if aspect of sub is different from video
            int heightOffset = 0;
            int targetHeight = subtitleHeight;
            if (optimalHeight)
            {
                heightOffset = (subtitleHeight - optimalHeight) / 2;
                targetHeight = optimalHeight;

                // if subtitle does not fit into optimal height, fall back to normal height
                if (maxH > optimalHeight + heightOffset || minY < heightOffset)
                {
                    optimalHeight = 0;
                    heightOffset = 0;
                    targetHeight = subtitleHeight;
                }
            }

            cueSize.Unit =TimedTextUnit::Percentage;
            cueSize.Width = (double)width * 100 / subtitleWidth;
            cueSize.Height = (double)height * 100 / targetHeight;

            // for some reason, all bitmap cues are moved down by 5% by uwp. we need to compensate for that.
            cuePosition.Unit =TimedTextUnit::Percentage;
            cuePosition.X = (double)offsetX * 100 / subtitleWidth;
            cuePosition.Y = ((double)(offsetY - heightOffset) * 100 / targetHeight) - 5;

            return true;
        }

        bool GetInitialSize()
        {
            if (!hasSize)
            {
                // initially get size information
                subtitleWidth = m_pAvCodecCtx->width;
                subtitleHeight = m_pAvCodecCtx->height;

                if (subtitleWidth > 0 && subtitleHeight > 0)
                {
                    if (subtitleWidth != videoWidth || subtitleHeight != videoHeight || (videoAspectRatio > 0 && videoAspectRatio != 1))
                    {
                        auto height = (int)(subtitleWidth / videoAspectRatio);
                        if (height < subtitleHeight)
                        {
                            optimalHeight = height;
                        }
                    }

                    hasSize = true;
                }
            }
            return hasSize;
        }

    private:
        int videoWidth = 0;
        int videoHeight = 0;
        double videoAspectRatio = 0.0;
        bool hasSize = false;
        int subtitleWidth = 0;
        int subtitleHeight = 0;
        int optimalHeight = 0;
        SoftwareBitmap dummyBitmap = { nullptr };

    };
}
