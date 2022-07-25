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
#include "StreamBuffer.h"
#include "UncompressedSampleProvider.h"

using namespace Concurrency;

using namespace FFmpegInteropX;

FFmpegReader::FFmpegReader(AVFormatContext* avFormatCtx, std::vector<shared_ptr<MediaSampleProvider>>* initProviders, MediaSourceConfig config)
    : avFormatCtx(avFormatCtx)
    , sampleProviders(initProviders)
    , config(config)
{
}

FFmpegReader::~FFmpegReader()
{
}

void FFmpegReader::Start()
{
    std::lock_guard lock(mutex);
    if (!isActive && (config.ReadAheadBufferEnabled() || config.ReadAheadBufferSize() > 0 || config.ReadAheadBufferDuration().count() > 0) && !config.as<implementation::MediaSourceConfig>()->IsFrameGrabber)
    {
        sleepTimerTarget = new call<int>([this](int value) { OnTimer(value); });
        if (!sleepTimerTarget)
        {
            return;
        }

        sleepTimer = new timer<int>(100u, 0, sleepTimerTarget, true);
        if (!sleepTimer)
        {
            delete sleepTimerTarget;
            return;
        }

        readTask = create_task([this]() { this->ReadDataLoop(); });
        isActive = true;
    }
}

void FFmpegReader::Stop()
{
    bool wait = false;
    {
        std::lock_guard lock(mutex);
        if (isActive)
        {
            isActive = false;
            wait = true;

            sleepTimer->stop();
        }
    }

    if (wait)
    {
        try
        {
            readTask.wait();
        }
        catch (...)
        {
            DebugMessage(L"Failed to wait for task. Probably destructor called from UI thread.\n");

            while (!readTask.is_done())
            {
                Sleep(1);
            }
        }
        delete sleepTimer;
        delete sleepTimerTarget;
    }

    if (forceReadStream != -1)
    {
        waitStreamEvent.set();
    }
    forceReadStream = -1;
    lastStream = nullptr;
    fullStream = nullptr;
}

void FFmpegReader::Flush()
{
    std::lock_guard lock(mutex);
    FlushCodecsAndBuffers();
}

void FFmpegReader::FlushCodecs()
{
    for (auto& stream : *sampleProviders)
    {
        if (stream)
            stream->Flush(false);
    }
}

void FFmpegReader::FlushCodecsAndBuffers()
{
    for (auto& stream : *sampleProviders)
    {
        if (stream)
            stream->Flush(true);
    }
    readResult = 0;
}

HRESULT FFmpegReader::Seek(TimeSpan position, TimeSpan& actualPosition, TimeSpan currentPosition, bool fastSeek, std::shared_ptr<MediaSampleProvider> videoStream, std::shared_ptr<MediaSampleProvider> audioStream)
{
    Stop();

    std::lock_guard lock(mutex);

    auto hr = S_OK;
    if (readResult != 0)
    {
        fastSeek = false;
        readResult = 0;
    }

    auto isForwardSeek = position > currentPosition;

    if (isForwardSeek && TrySeekBuffered(position, actualPosition, fastSeek, isForwardSeek, videoStream, audioStream))
    {
        // all good
        DebugMessage(L"BufferedSeek!\n");
    }
    else if (fastSeek)
    {
        hr = SeekFast(position, actualPosition, currentPosition, videoStream, audioStream);
    }
    else
    {
        DebugMessage(L"NormalSeek\n");
        // Select the first valid stream either from video or audio
        auto stream = videoStream ? videoStream : audioStream;
        int64_t seekTarget = stream->ConvertPosition(position);
        if (av_seek_frame(avFormatCtx, stream->StreamIndex(), seekTarget, AVSEEK_FLAG_BACKWARD) < 0)
        {
            hr = E_FAIL;
            DebugMessage(L" - ### Error while seeking\n");
        }
        else
        {
            // Flush all active streams with buffers
            FlushCodecsAndBuffers();
        }
    }

    return hr;
}

HRESULT FFmpegReader::SeekFast(TimeSpan position, TimeSpan& actualPosition, TimeSpan currentPosition, std::shared_ptr<MediaSampleProvider> videoStream, std::shared_ptr<MediaSampleProvider> audioStream)
{
    DebugMessage(L"SeekFast\n");

    HRESULT hr = S_OK;
    int64_t seekTarget = videoStream->ConvertPosition(position);
    bool isUriSource = avFormatCtx->url;

    // fast seek
    bool seekForward;
    TimeSpan referenceTime;

    // decide seek direction
    if (isLastSeekForward && position > lastSeekStart && position <= lastSeekActual)
    {
        seekForward = true;
        referenceTime = lastSeekStart + ((position - lastSeekStart) * 0.2);
        DebugMessage(L" - ### Forward seeking continue\n");
    }
    else if (!isLastSeekForward && position < lastSeekStart && position >= lastSeekActual)
    {
        seekForward = false;
        referenceTime = lastSeekStart + ((position - lastSeekStart) * 0.2);
        DebugMessage(L" - ### Backward seeking continue\n");
    }
    else if (position >= currentPosition)
    {
        seekForward = true;
        referenceTime = currentPosition + ((position - currentPosition) * 0.2);
        DebugMessage(L" - ### Forward seeking\n");
    }
    else
    {
        seekForward = false;
        referenceTime = currentPosition + ((position - currentPosition) * 0.2);
        DebugMessage(L" - ### Backward seeking\n");
    }

    int64_t min = INT64_MIN;
    int64_t max = INT64_MAX;
    if (seekForward)
    {
        min = videoStream->ConvertPosition(referenceTime);
    }
    else
    {
        max = videoStream->ConvertPosition(referenceTime);
    }

    if (avformat_seek_file(avFormatCtx, videoStream->StreamIndex(), min, seekTarget, max, 0) < 0)
    {
        hr = E_FAIL;
        DebugMessage(L" - ### Error while seeking\n");
    }
    else
    {
        // Flush all active streams with buffers
        FlushCodecsAndBuffers();

        // get and apply keyframe position for fast seeking
        TimeSpan timestampVideo;
        TimeSpan timestampVideoDuration;
        hr = videoStream->GetNextPacketTimestamp(timestampVideo, timestampVideoDuration);
        bool hasVideoPts = hr == S_OK;

        if (hr == S_FALSE)
        {
            // S_FALSE means that the video packets do not contain timestamps
            if (std::dynamic_pointer_cast<UncompressedSampleProvider>(videoStream))
            {
                // If we do not use passthrough, we can decode (and drop) then next sample to get the correct time.
                auto sample = videoStream->GetNextSample();
                if (sample)
                {
                    timestampVideo = sample.Timestamp();
                    timestampVideoDuration = sample.Duration();
                    timestampVideo += timestampVideoDuration;
                    hr = S_OK;
                }
            }
            else
            {
                // Otherwise, try with audio stream instead
                hr = audioStream->GetNextPacketTimestamp(timestampVideo, timestampVideoDuration);
            }
        }

        int seekCount = 0;
        while (hr == S_OK && seekForward && timestampVideo < referenceTime && !isUriSource && hasVideoPts && seekCount++ < 10)
        {
            // our min position was not respected. try again with higher min and target.
            min += videoStream->ConvertDuration(TimeSpan{ 50000000 });
            seekTarget += videoStream->ConvertDuration(TimeSpan{ 50000000 });

            if (avformat_seek_file(avFormatCtx, videoStream->StreamIndex(), min, seekTarget, max, 0) < 0)
            {
                hr = E_FAIL;
                DebugMessage(L" - ### Error while seeking\n");
            }
            else
            {
                // Flush all active streams with buffers
                FlushCodecsAndBuffers();

                // get updated timestamp
                hr = videoStream->GetNextPacketTimestamp(timestampVideo, timestampVideoDuration);
            }
        }

        if (hr == S_OK)
        {
            actualPosition = timestampVideo;

            // remember last seek direction
            isLastSeekForward = seekForward;
            lastSeekStart = position;
            lastSeekActual = actualPosition;

            if (audioStream)
            {
                // if we have audio, we need to seek back a bit more to get 100% clean audio
                TimeSpan timestampAudio;
                TimeSpan timestampAudioDuration;
                hr = audioStream->GetNextPacketTimestamp(timestampAudio, timestampAudioDuration);
                if (hr == S_OK)
                {
                    // audio stream should start one sample before video
                    auto audioTarget = timestampVideo - timestampAudioDuration;
                    auto audioPreroll = timestampAudio - timestampVideo;
                    if (audioPreroll.count() > 0 && config.FastSeekCleanAudio() && !isUriSource)
                    {
                        seekTarget = videoStream->ConvertPosition(audioTarget - audioPreroll);
                        if (av_seek_frame(avFormatCtx, videoStream->StreamIndex(), seekTarget, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_ANY) < 0)
                        {
                            hr = E_FAIL;
                            DebugMessage(L" - ### Error while seeking\n");
                        }
                        else
                        {
                            // Flush all active streams with buffers
                            FlushCodecsAndBuffers();

                            // Now drop all packets until desired keyframe position
                            videoStream->SkipPacketsUntilTimestamp(timestampVideo);
                            audioStream->SkipPacketsUntilTimestamp(audioTarget);

                            auto sample = audioStream->GetNextSample();
                            if (sample)
                            {
                                actualPosition = sample.Timestamp() + sample.Duration();
                            }
                        }
                    }
                    else if (audioPreroll.count() <= 0)
                    {
                        // Negative audio preroll. Just drop all packets until target position.
                        audioStream->SkipPacketsUntilTimestamp(audioTarget);

                        hr = audioStream->GetNextPacketTimestamp(timestampAudio, timestampAudioDuration);
                        if (hr == S_OK && (config.FastSeekCleanAudio() || (timestampAudio + timestampAudioDuration) <= timestampVideo))
                        {
                            // decode one audio sample to get clean output
                            auto sample = audioStream->GetNextSample();
                            if (sample)
                            {
                                actualPosition = sample.Timestamp() + sample.Duration();
                            }
                        }
                    }
                }
            }
        }
    }

    return hr;
}

bool FFmpegReader::TrySeekBuffered(TimeSpan position, TimeSpan& actualPosition, bool fastSeek, bool isForwardSeek, std::shared_ptr<MediaSampleProvider> videoStream, std::shared_ptr<MediaSampleProvider> audioStream)
{
    bool result = true;
    int vIndex = -1;
    int aIndex = -1;

    TimeSpan targetPosition = position;

    if (videoStream)
    {
        auto pts = videoStream->ConvertPosition(targetPosition);
        vIndex = videoStream->packetBuffer->TryFindPacketIndex(pts, true, fastSeek, isForwardSeek);
        result &= vIndex >= 0;

        if (result && fastSeek)
        {
            auto targetPacket = videoStream->packetBuffer->PeekPacketIndex(vIndex);
            if (targetPacket->pts != AV_NOPTS_VALUE)
            {
                targetPosition = videoStream->ConvertPosition(targetPacket->pts);
            }
        }
    }

    if (result && audioStream)
    {
        auto pts = audioStream->ConvertPosition(targetPosition);
        aIndex = audioStream->packetBuffer->TryFindPacketIndex(pts, false, fastSeek, isForwardSeek);
        result &= aIndex >= 0;
    }

    if (result && vIndex == 0)
    {
        // We are at correct position already. No flush required.
        DebugMessage(L"BufferedSeek: No flush\n");
    }
    else if (result)
    {
        // Flush all active streams but keep buffers
        FlushCodecs();

        if (videoStream)
        {
            videoStream->packetBuffer->DropPackets(vIndex);
        }

        if (audioStream)
        {
            if (config.FastSeekCleanAudio() && aIndex > 0)
            {
                aIndex--;
                audioStream->packetBuffer->DropPackets(aIndex);

                // decode one audio sample to get clean output
                auto sample = audioStream->GetNextSample();
                if (sample)
                {
                    // TODO check if this is a good idea
                    //actualPosition = sample.Timestamp() + sample.Duration();
                }
            }
            else
            {
                audioStream->packetBuffer->DropPackets(aIndex);
            }
        }

        actualPosition = targetPosition;
    }

    return result;
}

void FFmpegReader::ReadDataLoop()
{
    int ret = 0;
    bool sleep = false;

    while (true)
    {
        // Read next packet
        ret = ReadPacket();

        // Lock and check result
        std::lock_guard lock(mutex);
        if (!isActive)
        {
            // Stopped externally. No need to clean up.
            break;
        }
        else if (ret < 0 || !config.ReadAheadBufferEnabled())
        {
            // Self stop. Cleanup.
            isActive = false;
            sleepTimer->stop();
            delete sleepTimer;
            delete sleepTimerTarget;
            waitStreamEvent.set();
            break;
        }
        else
        {
            // Check if needs sleep
            sleep = CheckNeedsSleep(sleep);
            if (sleep)
            {
                isSleeping = true;
                sleepTimer->start();
                break;
            }
        }
    }
}

void FFmpegInteropX::FFmpegReader::OnTimer(int value)
{
    UNREFERENCED_PARAMETER(value);
    std::lock_guard lock(mutex);
    if (isActive)
    {
        readTask = create_task([this]() { ReadDataLoop(); });
        isSleeping = false;
    }
    sleepTimer->pause();
}

bool FFmpegReader::CheckNeedsSleep(bool wasSleeping)
{
    bool force = forceReadStream >= 0;
    if (force)
    {
        return false;
    }

    bool sleep = wasSleeping;

    // check if we need to start sleeping
    if (!sleep && lastStream && lastStream->IsBufferFull())
    {
        sleep = true;
        fullStream = lastStream;
    }
    // check if we can stop sleeping
    else if (sleep && !fullStream && !fullStream->IsBufferFull())
    {
        sleep = false;
        fullStream = nullptr;
        for (auto& stream : *sampleProviders)
        {
            if (stream && stream->IsBufferFull())
            {
                sleep = true;
                fullStream = stream;
                break;
            }
        }
    }

    return sleep;
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

    ret = av_read_frame(avFormatCtx, avPacket);
    std::lock_guard lock(mutex);
    readResult = ret;

    if (ret < 0)
    {
        av_packet_free(&avPacket);
    }
    else
    {
        if (avPacket->stream_index == forceReadStream)
        {
            forceReadStream = -1;
            waitStreamEvent.set();
        }

        if (avPacket->stream_index >= (int)sampleProviders->size())
        {
            // new stream detected. if this is a subtitle stream, we could create it now.
            av_packet_free(&avPacket);
        }
        else if (avPacket->stream_index < 0)
        {
            av_packet_free(&avPacket);
        }
        else
        {
            auto& provider = sampleProviders->at(avPacket->stream_index);
            if (provider)
            {
                provider->QueuePacket(avPacket);
                lastStream = provider;
            }
            else
            {
                DebugMessage(L"Ignoring unused stream\n");
                av_packet_free(&avPacket);
            }
        }
    }

    return readResult;
}


int FFmpegReader::ReadPacketForStream(StreamBuffer* buffer)
{
    if (!(buffer->IsEmpty()))
    {
        return 0;
    }

    bool manual;
    {
        std::lock_guard lock(mutex);
        manual = !isActive;
        if (readResult < 0)
        {
            return readResult;
        }
    }

    if (manual)
    {
        // no read-ahead used
        while (true)
        {
            ReadPacket();

            if (!(buffer->IsEmpty()))
            {
                break;
            }
            else if (readResult < 0)
            {
                break;
            }
        }
    }
    else
    {
        // read-ahead active
        while (true)
        {
            task<void> waitStreamTask;
            {
                std::lock_guard lock(mutex);

                if (!(buffer->IsEmpty()))
                {
                    forceReadStream = -1;
                    break;
                }
                else if (readResult < 0 || !isActive)
                {
                    break;
                }
                else
                {
                    forceReadStream = buffer->StreamIndex;
                    waitStreamEvent = task_completion_event<void>();
                    waitStreamTask = create_task(waitStreamEvent);
                }
            }

            waitStreamTask.wait();
        }
    }

    return readResult;
}
