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

#include "pch.h"
#include "FFmpegReader.h"
extern "C"
{
#include "libavformat/avformat.h"
}

using namespace FFmpegInteropX;

FFmpegInteropX::FFmpegReader::FFmpegReader(AVFormatContext* avFormatCtx, std::vector<std::shared_ptr<MediaSampleProvider>>* initProviders)
    : m_pAvFormatCtx(avFormatCtx)
    , sampleProviders(initProviders)
{
}

FFmpegInteropX::FFmpegReader::~FFmpegReader()
{
    DebugMessage(L"FFMpeg reader destroyed\n");
}

// Read the next packet from the stream and push it into the appropriate
// sample provider
int FFmpegInteropX::FFmpegReader::ReadPacket()
{
    int ret;
    AVPacket* avPacket = av_packet_alloc();
    if (!avPacket)
    {
        return E_OUTOFMEMORY;
    }

    ret = av_read_frame(m_pAvFormatCtx, avPacket);
    if (ret < 0)
    {
        av_packet_free(&avPacket);
        return ret;
    }
    AvSubtitleContextTrack* tempTrack = nullptr;
    if (avPacket->stream_index != 1 && avPacket->stream_index != 0)
    {
        tempTrack = nullptr;
    }
    //----------------------------------------------------------------------
    if (avSubtitleContextTracks_ptr != nullptr && avSubtitleContextTracks_ptr && avSubtitleContextTracks_ptr->size() > 0)
    {
        for (size_t i = 0; i < avSubtitleContextTracks_ptr->size(); i++)
        {
            auto track = avSubtitleContextTracks_ptr->at(i);
            if (track->index == avPacket->stream_index)
            {
                tempTrack = track;
            }
        }
    }
    //----------------------------------------------------------------------

    if (avPacket->stream_index >= (int)sampleProviders->size())
    {
        // new stream detected. if this is a subtitle stream, we could create it now.
        av_packet_free(&avPacket);
        return ret;
    }

    if (avPacket->stream_index < 0)
    {
        av_packet_free(&avPacket);
        return E_FAIL;
    }

    std::shared_ptr<MediaSampleProvider> provider = sampleProviders->at(avPacket->stream_index);


    if (tempTrack && tempTrack != nullptr && provider != nullptr)
    {
        //&& m_pSource != nullptr && m_pSource
        //&& (m_pSource->PrimarySubtitleIndex() == avPacket->stream_index || m_pSource->SecondarySubtitleIndex() == avPacket->stream_index))
        //auto avsub = winrt::FFmpegInteropX::ChapterInfo(codecName, lang, index, winrt::to_hstring(imageType));

        int got = 0;
        AVSubtitle avSubtitle;
        auto result = avcodec_decode_subtitle2(tempTrack->avSubtitleCodecCtx, &avSubtitle, &got, avPacket);
        auto framePts = avPacket->pts;
        auto frameDuration = avPacket->duration;
        if (got != 0)
        {
            TimeSpan position;
            TimeSpan duration;

            position = TimeSpan(LONGLONG(av_q2d(provider->m_pAvStream->time_base) * 10000000 * avPacket->pts) - provider->m_startOffset);
            duration = TimeSpan(LONGLONG(av_q2d(provider->m_pAvStream->time_base) * 10000000 * avPacket->duration));

            //m_FFmpegInteropMSS->RaiseSubtitleCueEntered(*avPacket, avSubtitle, tempTrack, position, duration);

            IBuffer buffer = nullptr;
            IBuffer buffer2 = nullptr;
            auto args = winrt::FFmpegInteropX::AvSubtitleEventArgs(position, duration, buffer, buffer2, 0, 1, 2, winrt::hstring());
            this->eventCallback(&args);

            avsubtitle_free(&avSubtitle);
            av_packet_free(&avPacket);
        }
        else
        {
            //DataWriter dataWriter = DataWriter();
            //auto aBuffer = new Platform::Array<uint8_t>(avPacket->data, avPacket->size);
            //dataWriter->WriteBytes(aBuffer);
            //auto random = RuntimeComponentUtils::Zip::GetDecompressed(dataWriter->DetachBuffer());
            //std::vector<uint8_t> vec(random->begin(), random->end());
            //uint8_t* newdata = vec.data();
            //avPacket->data = newdata;
            //avPacket->buf = nullptr;
            //avPacket->size = vec.size();

            //auto result2 = avcodec_decode_subtitle2(tempTrack->avSubtitleCodecCtx, &avSubtitle, &got, avPacket);
            //if (got)
            //{
            //    TimeSpan position;
            //    TimeSpan duration;

            //    position = TimeSpan(LONGLONG(av_q2d(provider->m_pAvStream->time_base) * 10000000 * avPacket->pts) - provider->m_startOffset);
            //    duration = TimeSpan(LONGLONG(av_q2d(provider->m_pAvStream->time_base) * 10000000 * avPacket->duration));

            //    //m_FFmpegInteropMSS->RaiseSubtitleCueEntered(*avPacket, avSubtitle, tempTrack, position, duration);
            //}

            avsubtitle_free(&avSubtitle);
            av_packet_free(&avPacket);
            return ret;
        }
    }
    else
    {
        if (provider)
        {
            provider->QueuePacket(avPacket);
        }
        else
        {
            DebugMessage(L"Ignoring unused stream\n");
            av_packet_free(&avPacket);
        }
    }


    return ret;
}
