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
#include "H264AVCSampleProvider.h"


H264AVCSampleProvider::H264AVCSampleProvider(
    std::shared_ptr<FFmpegReader> reader,
    AVFormatContext* avFormatCtx,
    AVCodecContext* avCodecCtx,
    MediaSourceConfig const& config,
    int streamIndex,
    VideoEncodingProperties encodingProperties,
    HardwareDecoderStatus hardwareDecoderStatus)
    : NALPacketSampleProvider(reader, avFormatCtx, avCodecCtx, config, streamIndex, encodingProperties, hardwareDecoderStatus)
{
}

H264AVCSampleProvider::~H264AVCSampleProvider()
{
}

HRESULT H264AVCSampleProvider::GetSPSAndPPSBuffer(DataWriter const& dataWriter, BYTE* buf, UINT32 length)
{
    HRESULT hr = S_OK;

    // Get the position of the SPS
    if (buf == nullptr || length < 7)
    {
        // The data isn't present
        return E_FAIL;
    }

    /* Extradata is in hvcC format */
    int pos = 4;
    m_nalLenSize = (buf[pos++] & 0x03) + 1;

    /* Decode SPS from hvcC. */
    int cnt = buf[pos++] & 0x1f;
    for (int i = 0; i < cnt; i++) {
        int nalsize = ReadMultiByteValue(buf, pos, 2);
        pos += 2;

        if (length - pos < (UINT32)nalsize) {
            return E_FAIL;
        }

        // Write the NAL unit to the stream
        dataWriter.WriteByte(0);
        dataWriter.WriteByte(0);
        dataWriter.WriteByte(0);
        dataWriter.WriteByte(1);

        auto bufferStart = buf + pos;
        auto buffer = winrt::array_view(bufferStart, nalsize);
        dataWriter.WriteBytes(buffer);

        pos += nalsize;
    }

    /* Decode PPS from hvcC. */
    cnt = buf[pos++];
    for (int i = 0; i < cnt; i++) {
        int nalsize = ReadMultiByteValue(buf, pos, 2);
        pos += 2;

        if (length - pos < (UINT32)nalsize) {
            return E_FAIL;
        }

        // Write the NAL unit to the stream
        dataWriter.WriteByte(0);
        dataWriter.WriteByte(0);
        dataWriter.WriteByte(0);
        dataWriter.WriteByte(1);

        auto bufferStart = buf + pos;
        auto buffer = winrt::array_view(bufferStart, nalsize);
        dataWriter.WriteBytes(buffer);

        pos += nalsize;
    }

    return hr;
}

// We cannot pass packet as-is. Use dataWriter approac
HRESULT H264AVCSampleProvider::WriteNALPacket(AVPacket* avPacket, IBuffer* pBuffer)
{
    auto dataWriter = DataWriter();
    auto hr = WriteNALPacketAfterExtradata(avPacket, dataWriter);
    if (SUCCEEDED(hr))
    {
        *pBuffer = dataWriter.DetachBuffer();
    }
    return hr;
}

// Write out an NAL packet converting stream offsets to start-codes
HRESULT H264AVCSampleProvider::WriteNALPacketAfterExtradata(AVPacket* avPacket, DataWriter const& dataWriter)
{
    HRESULT hr = S_OK;
    UINT32 index = 0;
    UINT32 size = 0;
    UINT32 packetSize = (UINT32)avPacket->size;

    do
    {
        // Make sure we have enough data
        if (packetSize < (index + m_nalLenSize))
        {
            hr = E_FAIL;
            break;
        }

        // Grab the size of the blob
        size = ReadMultiByteValue(avPacket->data, index, m_nalLenSize);
        index += m_nalLenSize;

        // Write the NAL unit to the stream
        dataWriter.WriteByte(0);
        dataWriter.WriteByte(0);
        dataWriter.WriteByte(0);
        dataWriter.WriteByte(1);

        // Stop if index and size goes beyond packet size or overflow
        if (packetSize < (index + size) || (UINT32_MAX - index) < size)
        {
            hr = E_FAIL;
            break;
        }

        // Write the rest of the packet to the stream
        auto bufferStart = avPacket->data + index;
        auto buffer = winrt::array_view(bufferStart, size);
        dataWriter.WriteBytes(buffer);
        index += size;
    } while (index < packetSize);

    return hr;
}

int H264AVCSampleProvider::ReadMultiByteValue(BYTE* buffer, int index, int numBytes)
{
    if (numBytes == 4)
    {
        return (buffer[index] << 24) + (buffer[index + 1] << 16) + (buffer[index + 2] << 8) + buffer[index + 3];
    }
    if (numBytes == 3)
    {
        return (buffer[index] << 16) + (buffer[index + 1] << 8) + buffer[index + 2];
    }
    if (numBytes == 2)
    {
        return (buffer[index] << 8) + buffer[index + 1];
    }
    if (numBytes == 1)
    {
        return (buffer[index]);
    }
    return -1;
}
