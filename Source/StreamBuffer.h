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

#include "FFmpegReader.h"
#include "MediaSourceConfig.h"
#include "MediaSampleProvider.h"

namespace FFmpegInteropX
{
    ref class FFmpegReader;

    ref class StreamBuffer
    {
    internal:
        StreamBuffer(int streamIndex, MediaSourceConfig^ config)
            : config(config)
        {
            StreamIndex = streamIndex;

        }

        property int StreamIndex;

        void QueuePacket(AVPacket* packet)
        {
            std::lock_guard<std::mutex> lock(mutex);
            buffer.push_back(packet);
            bufferSize += packet->size;
        }

        bool ReadUntilNotEmpty(FFmpegReader^ reader)
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

        bool SkipUntilTimestamp(FFmpegReader^ reader, LONGLONG target)
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
            std::lock_guard<std::mutex> lock(mutex);
            return buffer.empty();
        }

        bool IsFull(MediaSampleProvider^ sampleProvider)
        {
            std::lock_guard<std::mutex> lock(mutex);
            auto maxSize = config->ReadAheadBufferSize;
            auto maxDuration = config->ReadAheadBufferDuration;

            bool full = maxSize >= 0 && (long long)bufferSize > maxSize;
            if (!full && maxDuration.Duration >= 0 && buffer.size() > 1)
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
            std::lock_guard<std::mutex> lock(mutex);

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
            std::lock_guard<std::mutex> lock(mutex);

            AVPacket* packet = NULL;
            if (!buffer.empty())
            {
                packet = buffer.front();
            }

            return packet;
        }

        AVPacket* PeekPacketIndex(int index)
        {
            std::lock_guard<std::mutex> lock(mutex);
            return buffer.at(index);
        }

        int TryFindPacketIndex(LONGLONG pts, bool requireKeyFrame)
        {
            std::lock_guard<std::mutex> lock(mutex);

            bool hasEnd = false;
            int index = 0;
            int result = -1;
            for (auto packet : buffer)
            {
                if (packet->pts <= pts)
                {
                    if (!requireKeyFrame || packet->flags & AV_PKT_FLAG_KEY)
                    {
                        result = index;
                        if (packet->pts + packet->duration >= pts)
                        {
                            hasEnd = true;
                            break;
                        }
                    }
                }
                else
                {
                    hasEnd = true;
                    break;
                }
                index++;
            }

            if (result >= 0 && !hasEnd)
            {
                result = -1;
            }

            return result;
        }

        void Flush()
        {
            std::lock_guard<std::mutex> lock(mutex);
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
            std::lock_guard<std::mutex> lock(mutex);
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
        size_t bufferSize;
        MediaSourceConfig^ config;
    };
}
