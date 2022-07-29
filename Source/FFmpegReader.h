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

namespace FFmpegInteropX
{
    using namespace Concurrency;

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

        bool isFirstSeekAfterStreamSwitch = false;
        bool isLastSeekForward = false;
        TimeSpan lastSeekStart { 0 };
        TimeSpan lastSeekActual { 0 };
    };
}
