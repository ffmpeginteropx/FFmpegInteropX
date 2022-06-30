#pragma once

#include "SubtitleProvider.h"
#include <MemoryBuffer.h>

namespace FFmpegInteropX
{
    ref class SubtitleProviderBitmap : SubtitleProvider
    {

    internal:
        SubtitleProviderBitmap(FFmpegReader^ reader,
            AVFormatContext* avFormatCtx,
            AVCodecContext* avCodecCtx,
            MediaSourceConfig^ config,
            int index,
            CoreDispatcher^ dispatcher)
            : SubtitleProvider(reader, avFormatCtx, avCodecCtx, config, index, TimedMetadataKind::ImageSubtitle, dispatcher)
        {
        }

        virtual HRESULT Initialize() override
        {
            auto hr = SubtitleProvider::Initialize();

            if (SUCCEEDED(hr))
            {
                SubtitleTrack->CueEntered += ref new Windows::Foundation::TypedEventHandler<Windows::Media::Core::TimedMetadataTrack^, Windows::Media::Core::MediaCueEventArgs^>(this, &FFmpegInteropX::SubtitleProviderBitmap::OnCueEntered);
                SubtitleTrack->CueExited += ref new Windows::Foundation::TypedEventHandler<Windows::Media::Core::TimedMetadataTrack^, Windows::Media::Core::MediaCueEventArgs^>(this, &FFmpegInteropX::SubtitleProviderBitmap::OnCueExited);
            }

            return S_OK;
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

        virtual IMediaCue^ CreateCue(AVPacket* packet, TimeSpan* position, TimeSpan* duration) override
        {
            AVSubtitle* subtitle = (AVSubtitle*)malloc(sizeof(AVSubtitle));
            if (!subtitle)
            {
                return nullptr;
            }

            int gotSubtitle = 0;
            auto result = avcodec_decode_subtitle2(m_pAvCodecCtx, subtitle, &gotSubtitle, packet);
            if (result > 0 && gotSubtitle)
            {
                if (subtitle->start_display_time > 0)
                {
                    position->Duration += (long long)10000 * subtitle->start_display_time;
                }
                duration->Duration = (long long)10000 * subtitle->end_display_time;

                if (subtitle->num_rects <= 0)
                {
                    // inserty dummy cue
                    ImageCue^ cue = ref new ImageCue();
                    cue->SoftwareBitmap = GetDummyBitmap();
                    avsubtitle_free(subtitle);
                    delete subtitle;

                    return cue;
                }
                else
                {
                    int width, height, offsetX, offsetY;
                    TimedTextSize cueSize;
                    TimedTextPoint cuePosition;
                    if (subtitle->num_rects > 0 && CheckSize(subtitle, width, height, offsetX, offsetY, cueSize, cuePosition))
                    {
                        auto id = nextId++.ToString();
                        map[id] = subtitle;

                        ImageCue^ cue = ref new ImageCue();
                        cue->Id = id;
                        return cue;
                    }
                    else if (subtitle->num_rects > 0)
                    {
                        avsubtitle_free(subtitle);
                        delete subtitle;
                        OutputDebugString(L"Error: Invalid subtitle size received.");
                    }
                }
            }
            else if (result <= 0)
            {
                avsubtitle_free(subtitle);
                delete subtitle;
                OutputDebugString(L"Failed to decode subtitle.");
            }

            return nullptr;
        }

    public:

        void Flush(bool flushBuffers) override
        {
            SubtitleProvider::Flush(flushBuffers);

            if (!m_config->IsExternalSubtitleParser && flushBuffers)
            {
                for (auto entry : map)
                {
                    auto subtitle = entry.second;
                    avsubtitle_free(subtitle);
                    delete subtitle;
                }
                map.clear();
            }
        }

    private:

        void OnCueEntered(Windows::Media::Core::TimedMetadataTrack^ sender, Windows::Media::Core::MediaCueEventArgs^ args)
        {
            mutex.lock();
            try
            {
                //cleanup old cues to free memory
                std::vector<IMediaCue^> remove;
                for each (auto cue in SubtitleTrack->Cues)
                {
                    if (cue->StartTime + cue->Duration < args->Cue->StartTime)
                    {
                        remove.push_back(cue);
                    }
                }

                for each (auto cue in remove)
                {
                    SubtitleTrack->RemoveCue(cue);
                }

                auto cue = dynamic_cast<ImageCue^>(args->Cue);
                if (cue && cue->Id && !cue->SoftwareBitmap)
                {
                    auto subtitle = map.at(cue->Id);

                    int width, height, offsetX, offsetY;
                    TimedTextSize cueSize;
                    TimedTextPoint cuePosition;
                    if (subtitle->num_rects > 0 && CheckSize(subtitle, width, height, offsetX, offsetY, cueSize, cuePosition))
                    {
                        using namespace Windows::Graphics::Imaging;

                        auto bitmap = ref new SoftwareBitmap(BitmapPixelFormat::Bgra8, width, height, BitmapAlphaMode::Straight);
                        {
                            auto buffer = bitmap->LockBuffer(BitmapBufferAccessMode::Write);
                            auto reference = buffer->CreateReference();

                            // Query the IBufferByteAccess interface.  
                            Microsoft::WRL::ComPtr<Windows::Foundation::IMemoryBufferByteAccess> bufferByteAccess;
                            reinterpret_cast<IInspectable*>(reference)->QueryInterface(IID_PPV_ARGS(&bufferByteAccess));

                            // Retrieve the buffer data.  
                            byte* pixels = nullptr;
                            unsigned int capacity;
                            bufferByteAccess->GetBuffer(&pixels, &capacity);

                            auto plane = buffer->GetPlaneDescription(0);

                            for (unsigned int i = 0; i < subtitle->num_rects; i++)
                            {
                                auto rect = subtitle->rects[i];

                                for (int y = 0; y < rect->h; y++)
                                {
                                    for (int x = 0; x < rect->w; x++)
                                    {
                                        auto inPointer = rect->data[0] + y * rect->linesize[0] + x;
                                        auto color = inPointer[0];
                                        if (color < rect->nb_colors)
                                        {
                                            auto rgba = ((uint32*)rect->data[1])[color];
                                            auto outPointer = pixels + plane.StartIndex + plane.Stride * ((y + rect->y) - offsetY) + 4 * ((x + rect->x) - offsetX);
                                            ((uint32*)outPointer)[0] = rgba;
                                        }
                                        else
                                        {
                                            OutputDebugString(L"Error: Illegal subtitle color.");
                                        }
                                    }
                                }
                            }
                        }

                        cue->SoftwareBitmap = SoftwareBitmap::Convert(bitmap, BitmapPixelFormat::Bgra8, BitmapAlphaMode::Premultiplied);
                        cue->Position = cuePosition;
                        cue->Extent = cueSize;
                    }
                    else if (subtitle->num_rects > 0)
                    {
                        OutputDebugString(L"Error: Invalid subtitle size received.");
                    }
                }
            }
            catch (...)
            {
                OutputDebugString(L"Failed to render cue.");
            }
            mutex.unlock();
        }

        void OnCueExited(Windows::Media::Core::TimedMetadataTrack^ sender, Windows::Media::Core::MediaCueEventArgs^ args)
        {
            mutex.lock();
            try
            {
                auto cue = dynamic_cast<ImageCue^>(args->Cue);
                if (cue && cue->Id && cue->SoftwareBitmap)
                {
                    cue->SoftwareBitmap = GetDummyBitmap();
                }
            }
            catch (...)
            {
                OutputDebugString(L"Failed to cleanup old cues.");
            }
            mutex.unlock();
        }

        SoftwareBitmap^ GetDummyBitmap()
        {
            if (!dummyBitmap)
            {
                dummyBitmap = ref new SoftwareBitmap(BitmapPixelFormat::Bgra8, 16, 16, BitmapAlphaMode::Premultiplied);
            }

            return dummyBitmap;
        }

        bool CheckSize(AVSubtitle* subtitle, int& width, int& height, int& offsetX, int& offsetY, TimedTextSize& cueSize, TimedTextPoint& cuePosition)
        {
            if (!GetInitialSize())
            {
                return false;
            }

            // get actual extent of subtitle rects
            int minX = subtitleWidth, minY = subtitleHeight, maxW = 0, maxH = 0;
            for (unsigned int i = 0; i < subtitle->num_rects; i++)
            {
                auto rect = subtitle->rects[i];
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

            cueSize.Unit = TimedTextUnit::Percentage;
            cueSize.Width = (double)width * 100 / subtitleWidth;
            cueSize.Height = (double)height * 100 / targetHeight;

            // for some reason, all bitmap cues are moved down by 5% by uwp. we need to compensate for that.
            cuePosition.Unit = TimedTextUnit::Percentage;
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
        int videoWidth;
        int videoHeight;
        double videoAspectRatio;
        bool hasSize;
        int subtitleWidth;
        int subtitleHeight;
        int optimalHeight;
        SoftwareBitmap^ dummyBitmap;
        std::map<String^, AVSubtitle*> map;
        int nextId;
    };
}
