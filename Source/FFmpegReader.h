//*****************************************************************************
//
//	Copyright 2015 Microsoft Corporation
//
//	Licensed under the Apache License, Version 2.0 (the "License");
//	you may not use this file except in compliance with the License.
//	You may obtain a copy of the License at
//
//	http ://www.apache.org/licenses/LICENSE-2.0
//
//	Unless required by applicable law or agreed to in writing, software
//	distributed under the License is distributed on an "AS IS" BASIS,
//	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//	See the License for the specific language governing permissions and
//	limitations under the License.
//
//*****************************************************************************

#pragma once

#include "MediaSampleProvider.h"
#include "AvSubtitleContextTrack.h"

namespace FFmpegInteropX
{
    class FFmpegReader
    {
    public:
        virtual ~FFmpegReader();
        int ReadPacket();
        std::function<void(AvSubtitleEventArgs*)> eventCallback;
        std::shared_ptr<std::vector<AvSubtitleContextTrack*>> avSubtitleContextTracks_ptr;
        FFmpegReader(AVFormatContext* avFormatCtx, std::vector<std::shared_ptr<MediaSampleProvider>>* sampleProviders);

    private:
        AVFormatContext* m_pAvFormatCtx = NULL;
        std::vector<std::shared_ptr<MediaSampleProvider>>* sampleProviders = NULL;
        //----------------------------------------------------------------------
        void RaiseSubtitleCueEntered(AVPacket* avPacket, AVSubtitle* avSubtitle, AvSubtitleContextTrack* tempTrack, TimeSpan pos, TimeSpan dur)
        {
            if (tempTrack->m_avSubtitleCodec->id != AV_CODEC_ID_HDMV_PGS_SUBTITLE && tempTrack->m_avSubtitleCodec->id != AV_CODEC_ID_XSUB && tempTrack->m_avSubtitleCodec->id != AV_CODEC_ID_DVB_SUBTITLE)
            {
                auto index = avPacket->stream_index;
                DataWriter dataWriter = DataWriter();
                auto aBuffer = winrt::array_view<const uint8_t>(avPacket->data, avPacket->size);
                dataWriter.WriteBytes(aBuffer);
                auto args = winrt::FFmpegInteropX::AvSubtitleEventArgs(pos, dur, dataWriter.DetachBuffer(), nullptr, index, 0, 0, winrt::hstring(L"TEXT"));
                this->eventCallback(&args);
            }
            else
            {
                auto index = avPacket->stream_index;
                if (avSubtitle->rects != nullptr)
                {
                    DataWriter imageDataWriter = DataWriter();
                    DataWriter paletteDataWriter = DataWriter();
                    auto rect = (*avSubtitle->rects);
                    auto imageData = rect->data[0];
                    auto aBuffer = winrt::array_view<const uint8_t>(imageData, rect->w * rect->h);
                    imageDataWriter.WriteBytes(aBuffer);

                    auto palette = rect->data[1];
                    auto paletteBuffer = winrt::array_view<const uint8_t>(palette, rect->nb_colors * 4);
                    paletteDataWriter.WriteBytes(paletteBuffer);

                    auto args = winrt::FFmpegInteropX::AvSubtitleEventArgs(pos, dur, imageDataWriter.DetachBuffer(), paletteDataWriter.DetachBuffer(), index, rect->w, rect->h, winrt::hstring(L"IMAGE_SUBTITLE"));
                    this->eventCallback(&args);
                }
                else
                {
                    IBuffer buffer = nullptr;
                    IBuffer buffer2 = nullptr;
                    auto args = winrt::FFmpegInteropX::AvSubtitleEventArgs(pos, dur, buffer, buffer2, index, 0, 0, winrt::hstring(L"IMAGE_SUBTITLE"));
                    this->eventCallback(&args);
                }
            }
        }
        //----------------------------------------------------------------------
    };
}
