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

#include <agents.h>

#include "MediaSourceConfig.h"
#include "MediaSampleProvider.h"
#include "AvSubtitleContextTrack.h"

using namespace Concurrency;

extern "C"
{
#include "libavformat/avformat.h"
}
class StreamBuffer;

class FFmpegReader
{
public:
    FFmpegReader(AVFormatContext* avFormatCtx, std::vector<std::shared_ptr<MediaSampleProvider>>* sampleProviders, MediaSourceConfig config);
    virtual ~FFmpegReader();

    int ReadPacket();
    int ReadPacketForStream(StreamBuffer* buffer);
    void Start();
    void Stop();
    void Flush();
    HRESULT Seek(TimeSpan position, TimeSpan& actualPosition, TimeSpan currentPosition, bool allowFastSeek, std::shared_ptr<MediaSampleProvider> videoStream, std::shared_ptr<MediaSampleProvider> audioStream);

private:

    bool TrySeekBuffered(TimeSpan position, TimeSpan& actualPosition, bool fastSeek, bool isForwardSeek, std::shared_ptr<MediaSampleProvider> videoStream, std::shared_ptr<MediaSampleProvider> audioStream);
    HRESULT SeekFast(TimeSpan position, TimeSpan& actualPosition, TimeSpan currentPosition, std::shared_ptr<MediaSampleProvider> videoStream, std::shared_ptr<MediaSampleProvider> audioStream);
    void OnTimer(int value);
    void ReadDataLoop();
    void FlushCodecs();
    void FlushCodecsAndBuffers();
    bool CheckNeedsSleep(bool wasSleeping);

    AVFormatContext* avFormatCtx;
    std::vector<std::shared_ptr<MediaSampleProvider>>* sampleProviders{ nullptr };
    std::function<void(AvSubtitleEventArgs*)> eventCallback;
    std::function<void(int)> readCallback;
    std::shared_ptr<std::vector<AvSubtitleContextTrack*>> avSubtitleContextTracks_ptr;
    MediaSourceConfig config;
    std::shared_ptr<MediaSampleProvider> lastStream{ nullptr };
    std::shared_ptr<MediaSampleProvider> fullStream{ nullptr };

    std::recursive_mutex mutex;
    bool isActive = false;
    bool isSleeping = false;
    bool isEof = false;
    unsigned int errorCount = 0;
    int forceReadStream = 0;
    int readResult = 0;
    task<void> readTask;
    task_completion_event<void> waitStreamEvent;
    call<int>* sleepTimerTarget = NULL;
    timer<int>* sleepTimer = NULL;

    bool isLastSeekForward = false;
    TimeSpan lastSeekStart { 0 };
    TimeSpan lastSeekActual { 0 };

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
