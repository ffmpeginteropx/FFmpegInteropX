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
//#include "winrt/RuntimeComponentUtils.h"

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
    this->readCallback(avPacket->size);

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


    if (tempTrack && tempTrack != nullptr && provider != nullptr && this->eventCallback != nullptr)
    {
        //&& m_pSource != nullptr && m_pSource
        //&& (m_pSource->PrimarySubtitleIndex() == avPacket->stream_index || m_pSource->SecondarySubtitleIndex() == avPacket->stream_index))

        int got = 0;
        AVSubtitle avSubtitle;
        auto result = avcodec_decode_subtitle2(tempTrack->avSubtitleCodecCtx, &avSubtitle, &got, avPacket);
        auto framePts = avPacket->pts;
        auto frameDuration = avPacket->duration;
        if (got != 0)
        {
            TimeSpan position = TimeSpan(LONGLONG(av_q2d(provider->m_pAvStream->time_base) * 10000000 * avPacket->pts) - provider->m_startOffset);
            TimeSpan duration = TimeSpan(LONGLONG(av_q2d(provider->m_pAvStream->time_base) * 10000000 * avPacket->duration));
            this->RaiseSubtitleCueEntered(avPacket, &avSubtitle, tempTrack, position, duration);
            avsubtitle_free(&avSubtitle);
        }
        else
        {
            DebugMessage(L"Unable to decode subtitle stream, could be compressed\n");
            /* DataWriter dataWriter = DataWriter();
             auto aBuffer = winrt::array_view<const uint8_t>(avPacket->data, avPacket->size);
             dataWriter.WriteBytes(aBuffer);
             auto random = winrt::RuntimeComponentUtils::Zip::GetDecompressed(dataWriter.DetachBuffer());
             std::vector<uint8_t> vec(random.begin(), random.end());
             uint8_t* newdata = vec.data();
             avPacket->data = newdata;
             avPacket->buf = nullptr;
             avPacket->size = vec.size();

             auto result2 = avcodec_decode_subtitle2(tempTrack->avSubtitleCodecCtx, &avSubtitle, &got, avPacket);
             if (got)
             {
                 TimeSpan position = TimeSpan(LONGLONG(av_q2d(provider->m_pAvStream->time_base) * 10000000 * avPacket->pts) - provider->m_startOffset);
                 TimeSpan duration = TimeSpan(LONGLONG(av_q2d(provider->m_pAvStream->time_base) * 10000000 * avPacket->duration));
                 this->RaiseSubtitleCueEntered(avPacket, &avSubtitle, tempTrack, position, duration);
             }*/
            avsubtitle_free(&avSubtitle);
        }
    }

    if (provider)
    {
        provider->QueuePacket(avPacket);
    }
    else
    {
        DebugMessage(L"Ignoring unused stream\n");
        av_packet_free(&avPacket);
    }

    return ret;
}

