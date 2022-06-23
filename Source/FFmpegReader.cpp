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
#include "UncompressedSampleProvider.h"

using namespace Concurrency;

using namespace FFmpegInteropX;

FFmpegReader::FFmpegReader(AVFormatContext* avFormatCtx, std::vector<MediaSampleProvider^>* initProviders, MediaSourceConfig^ config)
	: avFormatCtx(avFormatCtx)
	, sampleProviders(initProviders)
    , config(config)
{
}

void FFmpegReader::Start()
{
	{
		std::lock_guard<std::mutex> lock(mutex);
		if (!isReading)
		{
			readTask = create_task([this] () { this->ReadDataLoop(); });
			isReading = true;
		}
	}
}

void FFmpegReader::Stop()
{
    bool wait = false;
    {
        std::lock_guard<std::mutex> lock(mutex);
        if (isReading)
        {
            isReading = false;
            wait = true;
        }
    }

    if (wait)
    {
        readTask.wait();
    }

}

void FFmpegReader::FlushCodecs()
{
    std::lock_guard<std::mutex> lock(mutex);
    for (auto stream : *sampleProviders)
    {
        stream->Flush(false);
    }
}

void FFmpegReader::Flush()
{
    std::lock_guard<std::mutex> lock(mutex);
    for (auto stream : *sampleProviders)
    {
        stream->Flush(true);
    }
    result = 0;
}

HRESULT FFmpegReader::Seek(TimeSpan position, TimeSpan& actualPosition, TimeSpan currentPosition, bool fastSeek, MediaSampleProvider^ videoStream, MediaSampleProvider^ audioStream)
{
    auto hr = S_OK;
    if (result != 0)
    {
        fastSeek = false;
        result = 0;
    }

    // Select the first valid stream either from video or audio
    auto stream = videoStream ? videoStream : audioStream;

    if (stream)
    {
        int64_t seekTarget = stream->ConvertPosition(position);

        TimeSpan videoSkipPts, audioSkipPts;
        if (TrySeekBuffered(position, actualPosition, fastSeek, videoStream, audioStream))
        {
            // Flush all active stream codecs but keep bufferss
            FlushCodecs();
        }
        else if (fastSeek)
        {
            hr = SeekFast(position, actualPosition, currentPosition, videoStream, audioStream);
        }
        else
        {
            if (av_seek_frame(avFormatCtx, stream->StreamIndex, seekTarget, AVSEEK_FLAG_BACKWARD) < 0)
            {
                hr = E_FAIL;
                DebugMessage(L" - ### Error while seeking\n");
            }
            else
            {
                // Flush all active streams with buffers
                Flush();
            }
        }
    }
    else
    {
        hr = E_FAIL;
    }

    return hr;
}

HRESULT FFmpegReader::SeekFast(TimeSpan position, TimeSpan& actualPosition, TimeSpan currentPosition, MediaSampleProvider^ videoStream, MediaSampleProvider^ audioStream)
{
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

    if (avformat_seek_file(avFormatCtx, videoStream->StreamIndex, min, seekTarget, max, 0) < 0)
    {
        hr = E_FAIL;
        DebugMessage(L" - ### Error while seeking\n");
    }
    else
    {
        // Flush all active streams with buffers
        Flush();

        // get and apply keyframe position for fast seeking
        TimeSpan timestampVideo;
        TimeSpan timestampVideoDuration;
        hr = videoStream->GetNextPacketTimestamp(timestampVideo, timestampVideoDuration);

        if (hr == S_FALSE && isUriSource)
        {
            if (dynamic_cast<UncompressedSampleProvider^>(videoStream))
            {
                auto sample = videoStream->GetNextSample();
                if (sample)
                {
                    timestampVideo = sample->Timestamp;
                    timestampVideoDuration = sample->Duration;
                    timestampVideo += timestampVideoDuration;
                    hr = S_OK;
                }
            }
            else
            {
                //hr = audioStream->GetNextPacketTimestamp(timestampVideo, timestampVideoDuration);
            }
        }

        while (hr == S_OK && seekForward && timestampVideo < referenceTime && !isUriSource)
        {
            // our min position was not respected. try again with higher min and target.
            min += videoStream->ConvertDuration(TimeSpan{ 50000000 });
            seekTarget += videoStream->ConvertDuration(TimeSpan{ 50000000 });

            if (avformat_seek_file(avFormatCtx, videoStream->StreamIndex, min, seekTarget, max, 0) < 0)
            {
                hr = E_FAIL;
                DebugMessage(L" - ### Error while seeking\n");
            }
            else
            {
                // Flush all active streams with buffers
                Flush();

                // get updated timestamp
                hr = videoStream->GetNextPacketTimestamp(timestampVideo, timestampVideoDuration);
            }
        }

        if (hr == S_OK)
        {
            // reduce playback delay
            timestampVideo += timestampVideoDuration;

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
                    if (audioPreroll.Duration > 0 && config->FastSeekCleanAudio && !isUriSource)
                    {
                        seekTarget = videoStream->ConvertPosition(audioTarget - audioPreroll);
                        if (av_seek_frame(avFormatCtx, videoStream->StreamIndex, seekTarget, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_ANY) < 0)
                        {
                            hr = E_FAIL;
                            DebugMessage(L" - ### Error while seeking\n");
                        }
                        else
                        {
                            // Flush all active streams with buffers
                            Flush();

                            // Now drop all packets until desired keyframe position
                            videoStream->SkipPacketsUntilTimestamp(timestampVideo);
                            audioStream->SkipPacketsUntilTimestamp(audioTarget);

                            auto sample = audioStream->GetNextSample();
                            /*if (sample)
                            {
                                actualPosition = sample->Timestamp + sample->Duration;
                            }*/
                        }
                    }
                    else if (audioPreroll.Duration <= 0)
                    {
                        // Negative audio preroll. Just drop all packets until target position.
                        audioStream->SkipPacketsUntilTimestamp(audioTarget);

                        hr = audioStream->GetNextPacketTimestamp(timestampAudio, timestampAudioDuration);
                        if (hr == S_OK && (config->FastSeekCleanAudio || (timestampAudio + timestampAudioDuration) <= timestampVideo))
                        {
                            // decode one audio sample to get clean output
                            auto sample = audioStream->GetNextSample();
                            /*if (sample)
                            {
                                actualPosition = sample->Timestamp + sample->Duration;
                            }*/
                        }
                    }
                }
            }
        }
    }

    return hr;
}

bool FFmpegReader::TrySeekBuffered(TimeSpan position, TimeSpan& actualPosition, bool fastSeek, MediaSampleProvider^ videoStream, MediaSampleProvider^ audioStream)
{
    bool result = true;;
    int vIndex; int aIndex;

    TimeSpan targetPosition = position;

    if (videoStream)
    {
        auto pts = videoStream->ConvertPosition(targetPosition);
        vIndex = videoStream->buffer->TryFindPacketIndex(pts, true);
        result &= vIndex >= 0;

        if (result && fastSeek)
        {
            auto targetPacket = videoStream->buffer->PeekPacketIndex(vIndex);
            if (targetPacket->pts != AV_NOPTS_VALUE)
            {
                targetPosition = videoStream->ConvertPosition(targetPacket->pts);
            }
        }
    }

    if (result && audioStream)
    {
        auto pts = audioStream->ConvertPosition(targetPosition);
        aIndex = audioStream->buffer->TryFindPacketIndex(pts, false);
        result &= aIndex >= 0;
    }

    if (result)
    {
        // Flush all active streams but keep buffers
        FlushCodecs();

        if (videoStream)
        {
            videoStream->buffer->DropPackets(vIndex);
        }

        if (audioStream)
        {
            if (config->FastSeekCleanAudio && aIndex > 0)
            {
                aIndex--;
                audioStream->buffer->DropPackets(aIndex);

                // decode one audio sample to get clean output
                auto sample = audioStream->GetNextSample();
                if (sample)
                {
                    actualPosition = sample->Timestamp + sample->Duration;
                }
            }
            else
            {
                audioStream->buffer->DropPackets(aIndex);
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
    bool force = false;

	while (true)
	{
		{
			std::lock_guard<std::mutex> lock(mutex);
            force = forceReadStream >= 0;
            sleep = false;
            for (auto stream : *sampleProviders)
            {
                sleep &= stream->buffer->IsFull(stream);
            }
			if (!isReading)
			{
                break;
			}
		}

		if (!sleep || force)
		{
            ret = ReadPacket();
            if (ret < 0)
            {
                break;
            }
		}
		else
		{
			Sleep(100);
		}
	}

	{
		std::lock_guard<std::mutex> lock(mutex);
		isReading = false;
        forceReadStream = -1;
        result = ret;
	}
}

FFmpegReader::~FFmpegReader()
{
}

// Read the next packet from the stream and push it into the appropriate
// sample provider
int FFmpegReader::ReadPacket()
{
    int ret;
    AVPacket* avPacket = av_packet_alloc();

    if (!avPacket)
    {
        ret = E_OUTOFMEMORY;
    }

    ret = av_read_frame(avFormatCtx, avPacket);
    if (ret < 0)
    {
        std::lock_guard<std::mutex> lock(mutex);
        av_packet_free(&avPacket);
    }
    else
    {
        {
            std::lock_guard<std::mutex> lock(mutex);

            if (avPacket->stream_index == forceReadStream)
            {
                forceReadStream = -1;
            }

            if (avPacket->stream_index >= (int)sampleProviders->size())
            {
                // new stream detected. if this is a subtitle stream, we could create it now.
                av_packet_free(&avPacket);
            }

            if (avPacket->stream_index < 0)
            {
                av_packet_free(&avPacket);
            }

            MediaSampleProvider^ provider = sampleProviders->at(avPacket->stream_index);
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
    }

    result = ret;
    return result;
}


int FFmpegReader::ReadPacketForStream(StreamBuffer^ buffer)
{
    bool manual;
    {
        std::lock_guard<std::mutex> lock(mutex);
        manual = !isReading;
    }

	while (true)
	{
		{
			std::lock_guard<std::mutex> lock(mutex);

			if (!(buffer->IsEmpty()))
			{
                forceReadStream = -1;
                break;
			}
            else if (result < 0)
            {
                break;
            }
            else
            {
                forceReadStream = buffer->StreamIndex;
            }
		}

        //task_completion_event<int> tce;

        if (manual)
        {
            ReadPacket();
        }
        else
        {
            Sleep(10);
        }
	}

	return result;
}

