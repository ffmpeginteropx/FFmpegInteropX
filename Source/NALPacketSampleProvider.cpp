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
#include "NALPacketSampleProvider.h"

using namespace FFmpegInteropX;
using namespace winrt::Windows::Storage::Streams;

NALPacketSampleProvider::NALPacketSampleProvider(
    std::shared_ptr<FFmpegReader> reader,
    AVFormatContext* avFormatCtx,
    AVCodecContext* avCodecCtx,
    MediaSourceConfig const& config,
    int streamIndex,
    VideoEncodingProperties const& pencodingProperties,
    HardwareDecoderStatus hardwareDecoderStatus)
    : CompressedSampleProvider(reader, avFormatCtx, avCodecCtx, config, streamIndex, pencodingProperties, hardwareDecoderStatus)
{
}

NALPacketSampleProvider::~NALPacketSampleProvider()
{
}

void NALPacketSampleProvider::Flush()
{
    CompressedSampleProvider::Flush();
    m_bHasSentExtradata = false;
}

HRESULT NALPacketSampleProvider::CreateBufferFromPacket(AVPacket* avPacket, IBuffer* pBuffer)
{
    HRESULT hr = S_OK;
    DataWriter dataWriter = nullptr;

    // On first frame, write the SPS and PPS
    if (!m_bHasSentExtradata)
    {
        dataWriter = DataWriter();
        hr = GetSPSAndPPSBuffer(dataWriter, m_pAvCodecCtx->extradata, m_pAvCodecCtx->extradata_size);
        m_bHasSentExtradata = true;
    }

    if (SUCCEEDED(hr))
    {
        // Check for extradata changes during playback
        for (int i = 0; i < avPacket->side_data_elems; i++)
        {
            if (avPacket->side_data[i].type == AV_PKT_DATA_NEW_EXTRADATA && avPacket->side_data[i].size < UINT32_MAX)
            {
                if (dataWriter == nullptr)
                {
                    dataWriter = DataWriter();
                }
                hr = GetSPSAndPPSBuffer(dataWriter, avPacket->side_data[i].data, (UINT32)avPacket->side_data[i].size);
                break;
            }
        }

    }

    if (SUCCEEDED(hr))
    {
        if (dataWriter == nullptr)
        {
            // no extradata: write out packet as-is
            hr = WriteNALPacket(avPacket, pBuffer);
        }
        else
        {
            // append packet after extradata
            hr = WriteNALPacketAfterExtradata(avPacket, dataWriter);

            if (SUCCEEDED(hr))
            {
                *pBuffer = dataWriter.DetachBuffer();
            }
        }
    }

    return hr;
}

HRESULT NALPacketSampleProvider::GetSPSAndPPSBuffer(DataWriter const& dataWriter, BYTE* buf, UINT32 length)
{
    HRESULT hr = S_OK;

    if (buf == nullptr || length < 8)
    {
        // The data isn't present
        hr = E_FAIL;
    }
    else
    {
        // Write both SPS and PPS sequence as is from extradata
        auto buffer = winrt::array_view(buf, length);
        dataWriter.WriteBytes(buffer);
    }

    return hr;
}

HRESULT NALPacketSampleProvider::WriteNALPacketAfterExtradata(AVPacket* avPacket, DataWriter const& dataWriter)
{
    // Write out the NAL packet
    auto buffer = winrt::array_view(avPacket->data, avPacket->size);
    dataWriter.WriteBytes(buffer);
    return S_OK;
}

HRESULT NALPacketSampleProvider::WriteNALPacket(AVPacket* avPacket, IBuffer* pBuffer)
{
    // Write out the NAL packet as-is
    return CompressedSampleProvider::CreateBufferFromPacket(avPacket, pBuffer);
}
