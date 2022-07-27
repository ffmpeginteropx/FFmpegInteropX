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
    winrt::FFmpegInteropX::ChapterInfo* tempTrack;
    if (avPacket->stream_index != 1 && avPacket->stream_index != 0)
    {
        tempTrack = nullptr;
    }
    //----------------------------------------------------------------------
    if (avSubtitleTracks_ptr && avSubtitleTracks_ptr->size() > 0)
    {
        for (size_t i = 0; i < avSubtitleTracks_ptr->size(); i++)
        {
            auto track = avSubtitleTracks_ptr->at(i);
            auto index = (int)track.Index();
            if (index == avPacket->stream_index)
            {
                tempTrack = &track;
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
