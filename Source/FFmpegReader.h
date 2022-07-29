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
                //SubtitleCueEventArgs^ cue = ref new SubtitleCueEventArgs();
                if (avSubtitle->rects != nullptr)
                {
                    //DataWriter^ ImageDataWriter = ref new DataWriter();
                    //DataWriter^ paletteDataWriter = ref new DataWriter();

                    auto imageData = (*avSubtitle->rects)->data[0];
                  /*  auto aBuffer = ref new Platform::Array<uint8_t>(imageData, (*avSubtitle.rects)->w * (*avSubtitle.rects)->h);
                    ImageDataWriter->WriteBytes(aBuffer);
                    auto palette = (*avSubtitle.rects)->data[1];
                    auto paletteBuffer = ref new Platform::Array<uint8_t>(palette, (*avSubtitle.rects)->nb_colors * 4);
                    paletteDataWriter->WriteBytes(paletteBuffer);


                    cue->_index = avPacket.stream_index;
                    cue->_starttime = pos.Duration / 10000;
                    cue->_duration = dur.Duration / 10000;
                    cue->_width = (*avSubtitle.rects)->w;
                    cue->_height = (*avSubtitle.rects)->h;
                    cue->_type = ref new Platform::String(L"IMAGE_SUBTITLE");
                    cue->_ibuffer = ImageDataWriter->DetachBuffer();
                    cue->_ibuffer2 = paletteDataWriter->DetachBuffer();
                    SubtitleCueEntered(this, cue);*/
                }
                else
                {
                    auto index = avPacket->stream_index;
                    IBuffer buffer = nullptr;
                    IBuffer buffer2 = nullptr;
                    auto args = winrt::FFmpegInteropX::AvSubtitleEventArgs(pos, dur, buffer, buffer2, index, 0, 0, winrt::hstring(L"IMAGE_SUBTITLE"));
                    this->eventCallback(&args);
                   /* cue->_index = avPacket.stream_index;
                    cue->_starttime = pos.Duration / 10000;
                    cue->_duration = dur.Duration / 10000;
                    cue->_type = ref new Platform::String(L"IMAGE_SUBTITLE");
                    SubtitleCueEntered(this, cue);*/
                }
            }

            /*      auto args = winrt::FFmpegInteropX::AvSubtitleEventArgs(position, duration, buffer, buffer2, 0, 1, 2, winrt::hstring());
      this->eventCallback(&args);*/
        }

    };
}
