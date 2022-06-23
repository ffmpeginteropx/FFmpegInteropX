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

#include <deque>
#include <ppltasks.h>

#include "MediaSourceConfig.h"
#include "MediaSampleProvider.h"

namespace FFmpegInteropX
{
    using namespace Concurrency;

    ref class StreamBuffer;

    ref class FFmpegReader
    {
    internal:
        FFmpegReader(AVFormatContext* avFormatCtx, std::vector<MediaSampleProvider^>* sampleProviders, MediaSourceConfig^ config);

        int ReadPacket();
        int ReadPacketForStream(StreamBuffer^ buffer);
        void Start();
        void Stop();
        void Flush();
        HRESULT Seek(TimeSpan position, TimeSpan& actualPosition, TimeSpan currentPosition, bool allowFastSeek, MediaSampleProvider^ videoStream, MediaSampleProvider^ audioStream);


    private:

        ~FFmpegReader();
        bool TrySeekBuffered(TimeSpan position, TimeSpan& actualPosition, bool fastSeek, MediaSampleProvider^ videoStream, MediaSampleProvider^ audioStream);
        HRESULT SeekFast(TimeSpan position, TimeSpan& actualPosition, TimeSpan currentPosition, MediaSampleProvider^ videoStream, MediaSampleProvider^ audioStream);
        void ReadDataLoop();
        void FlushCodecs();

        AVFormatContext* avFormatCtx;
        std::vector<MediaSampleProvider^>* sampleProviders;
        MediaSourceConfig^ config;

        std::mutex mutex;
        bool isReading;
        int forceReadStream;
        int result;
        task<void> readTask;

        bool isFirstSeekAfterStreamSwitch;
        bool isLastSeekForward;
        TimeSpan lastSeekStart;
        TimeSpan lastSeekActual;
    };
}
