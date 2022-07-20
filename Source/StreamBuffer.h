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

namespace FFmpegInteropX
{
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
            auto maxSize = config.ReadAheadBufferSize();
            auto maxDuration = config.ReadAheadBufferDuration();

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

        int TryFindPacketIndex(LONGLONG pts, bool requireKeyFrame, bool fastSeek)
        {
            std::lock_guard lock(mutex);

            if (buffer.size() == 0)
            {
                return -1;
            }

            auto firstPacket = buffer.front();
            auto lastPacket = buffer.back();
            auto firstPts = firstPacket->pts != AV_NOPTS_VALUE ? firstPacket->pts : firstPacket->dts;
            auto lastPts = lastPacket->pts != AV_NOPTS_VALUE ? lastPacket->pts : lastPacket->dts;

            if (firstPts != AV_NOPTS_VALUE && lastPts != AV_NOPTS_VALUE && (firstPts > pts || lastPts < pts))
            {
                return -1;
            }

            bool hasEnd = false;
            int index = 0;
            int result = -1;
            for (auto packet : buffer)
            {
                if (packet->pts != AV_NOPTS_VALUE)
                {
                    // with fast seek, we can select a key frame behind seek target
                    if (packet->pts <= pts || (fastSeek && result == -1))
                    {
                        if (!requireKeyFrame || packet->flags & AV_PKT_FLAG_KEY)
                        {
                            result = index;
                        }
                    }

                    if (packet->pts + packet->duration >= pts && (!fastSeek || result > -1))
                    {
                        hasEnd = true;
                        break;
                    }
                }
                index++;
            }

            if (hasEnd && result == -1 && !fastSeek)
            {
                // no key frame between current frame and target position. need to decode from here.
                result = 0;
            }

            if (result >= 0 && !hasEnd)
            {
                // key frames found but buffer range not up to target position
                result = -1;
            }

            return result;
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
}
