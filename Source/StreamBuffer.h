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

#include "FFmpegReader.h"
#include "MediaSourceConfig.h"
#include "MediaSampleProvider.h"

class FFmpegReader;

class StreamBuffer
{
public:
    StreamBuffer(int streamIndex, winrt::FFmpegInteropX::MediaSourceConfig const& config)
        : config(config)
    {
        StreamIndex = streamIndex;
    }

    int StreamIndex;

    void QueuePacket(AVPacket* packet)
    {
        std::lock_guard lock(mutex);
        buffer.push_back(packet);
        bufferSize += packet->size;
    }

    bool ReadUntilNotEmpty(std::shared_ptr<FFmpegReader> reader)
    {
        while (IsEmpty())
        {
            if (reader->ReadPacketForStream(this) < 0)
            {
                DebugMessage(L"GetNextPacket reaching EOF\n");
                break;
            }
        }
        return !IsEmpty();
    }

    bool SkipUntilTimestamp(std::shared_ptr<FFmpegReader> reader, LONGLONG target)
    {
        bool foundPacket = false;

        while (!foundPacket)
        {
            if (ReadUntilNotEmpty(reader))
            {
                // peek next packet and check pts value
                auto packet = PeekPacket();

                auto pts = packet->pts != AV_NOPTS_VALUE ? packet->pts : packet->dts;
                if (pts != AV_NOPTS_VALUE && packet->duration != AV_NOPTS_VALUE)
                {
                    auto packetEnd = pts + packet->duration;
                    if (packet->duration > 0 ? packetEnd <= target : packetEnd < target)
                    {
                        DropPackets(1);
                    }
                    else
                    {
                        foundPacket = true;
                        break;
                    }
                }
                else
                {
                    break;
                }
            }
            else
            {
                // no more packet found
                break;
            }
        }

        return foundPacket;
    }

    bool IsEmpty()
    {
        std::lock_guard lock(mutex);
        return buffer.empty();
    }

    bool IsFull(MediaSampleProvider* sampleProvider)
    {
        std::lock_guard lock(mutex);
        if (buffer.empty())
        {
            return false;
        }
        auto maxSize = config.General().ReadAheadBufferSize();
        auto maxDuration = config.General().ReadAheadBufferDuration();

        bool full = maxSize >= 0 && (long long)bufferSize > maxSize;
        if (!full && maxDuration.count() >= 0 && buffer.size() > 1)
        {
            auto firstPacket = buffer.front();
            auto lastPacket = buffer.back();
            auto firstPts = firstPacket->pts != AV_NOPTS_VALUE ? firstPacket->pts : firstPacket->dts;
            auto lastPts = lastPacket->pts != AV_NOPTS_VALUE ? lastPacket->pts : lastPacket->dts;

            if (firstPts != AV_NOPTS_VALUE && lastPts != AV_NOPTS_VALUE)
            {
                auto duration = sampleProvider->ConvertDuration(lastPts - firstPts);
                full = duration > maxDuration;
            }
        }
        return full;
    }

    AVPacket* PopPacket()
    {
        std::lock_guard lock(mutex);

        AVPacket* packet = NULL;
        if (!buffer.empty())
        {
            packet = buffer.front();
            buffer.pop_front();
            bufferSize -= packet->size;
        }

        return packet;
    }

    AVPacket* PeekPacket()
    {
        std::lock_guard lock(mutex);

        AVPacket* packet = NULL;
        if (!buffer.empty())
        {
            packet = buffer.front();
        }

        return packet;
    }

    AVPacket* PeekPacketIndex(int index)
    {
        std::lock_guard lock(mutex);
        return buffer.at(index);
    }

    int TryFindPacketIndex(LONGLONG pts, LONGLONG& resultPts, bool requireKeyFrame, bool fastSeek, bool isForwardSeek)
    {
        std::lock_guard lock(mutex);

        if (buffer.size() == 0)
        {
            return -1;
        }

        auto firstPacket = buffer.front();
        auto lastPacket = buffer.back();
        auto firstPts = GetTimestamp(firstPacket);
        auto lastPts = GetTimestamp(lastPacket);

        if (firstPts != AV_NOPTS_VALUE && lastPts != AV_NOPTS_VALUE && (firstPts > pts || lastPts < pts))
        {
            return -1;
        }

        if (requireKeyFrame)
        {
            return TryFindClosestKeyframe(pts, isForwardSeek, fastSeek, resultPts);
        }
        else
        {
            return TryFindClosestPacket(pts);
        }
    }

    int TryFindClosestPacket(long long target)
    {
        int index = 0;
        int result = -1;
        for (auto packet : buffer)
        {
            auto pts = GetTimestamp(packet);
            if (pts != AV_NOPTS_VALUE)
            {
                if (pts + packet->duration >= target)
                {
                    result = index;
                    break;
                }
            }
            index++;
        }
        return result;
    }

    int TryFindClosestKeyframe(long long target, bool isForwardSeek, bool fastSeek, LONGLONG& resultPts)
    {
        bool hasTarget = false;
        int index = 0;
        int packetBeforeIndex = -1;
        int packetAfterIndex = -1;
        long long packetBeforePts = -1;
        long long packetAfterPts = -1;
        long long lastPacketPts = AV_NOPTS_VALUE;
        for (auto packet : buffer)
        {
            auto pts = GetTimestamp(packet);
            if (pts == AV_NOPTS_VALUE && packet->flags & AV_PKT_FLAG_KEY)
            {
                // in some streams, key frames do not have pts/dts. use previous packet value instead.
                pts = lastPacketPts;
            }
            if (pts != AV_NOPTS_VALUE)
            {
                if (pts <= target && packet->flags & AV_PKT_FLAG_KEY)
                {
                    packetBeforeIndex = index;
                    packetBeforePts = pts;
                }

                if (pts + packet->duration >= target)
                {
                    hasTarget = true;
                    if (packet->flags & AV_PKT_FLAG_KEY)
                    {
                        packetAfterIndex = index;
                        packetAfterPts = pts;
                        break;
                    }
                }
            }
            lastPacketPts = pts;
            index++;
        }

        if (hasTarget)
        {
            if (!fastSeek)
            {
                // no fast seek: use packet before or decode from current position
                if (packetBeforeIndex >= 0)
                {
                    return packetBeforeIndex;
                }
                else if (hasTarget)
                {
                    return 0;
                }
                else
                {
                    return -1;
                }
            }
            else
            {
                if (packetBeforeIndex >= 0 && packetAfterIndex >= 0)
                {
                    // keyframes before and after found. select closest.
                    auto diffBefore = target - packetBeforePts;
                    auto diffAfter = packetAfterPts - target;
                    if (diffBefore <= diffAfter)
                    {
                        resultPts = packetBeforePts;
                        return packetBeforeIndex;
                    }
                    else
                    {
                        resultPts = packetAfterPts;
                        return packetAfterIndex;
                    }
                }
                else if (packetBeforeIndex >= 0)
                {
                    // only keyframe before position found. return it.
                    resultPts = packetBeforePts;
                    return packetBeforeIndex;
                }
                else
                {
                    // only keyframe after position found. use it or continue from current position.
                    auto diffCurrent = target - GetTimestamp(buffer[0]);
                    auto diffAfter = packetAfterPts - target;
                    if (diffCurrent < diffAfter && !isForwardSeek)
                    {
                        return 0;
                    }
                    else
                    {
                        resultPts = packetAfterPts;
                        return packetAfterIndex;
                    }
                }
            }
        }
        else
        {
            // target not found in buffer range
            return -1;
        }
    }

    long long GetTimestamp(AVPacket* packet)
    {
        return packet->pts != AV_NOPTS_VALUE ? packet->pts : packet->dts;
    }

    void Flush()
    {
        std::lock_guard lock(mutex);
        while (!buffer.empty())
        {
            auto packet = buffer.front();
            bufferSize -= packet->size;
            buffer.pop_front();

            av_packet_free(&packet);
        }
    }

    void DropPackets(int count)
    {
        std::lock_guard lock(mutex);
        for (int i = 0; i < count; i++)
        {
            auto packet = buffer.front();
            bufferSize -= packet->size;
            buffer.pop_front();

            av_packet_free(&packet);
        }
    }

private:
    std::deque<AVPacket*> buffer;
    std::mutex mutex;
    size_t bufferSize = 0;
    winrt::FFmpegInteropX::MediaSourceConfig config{nullptr};
};
