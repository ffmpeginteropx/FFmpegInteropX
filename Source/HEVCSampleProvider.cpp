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
#include "HEVCSampleProvider.h"


HEVCSampleProvider::HEVCSampleProvider(
    std::shared_ptr<FFmpegReader> reader,
    AVFormatContext* avFormatCtx,
    AVCodecContext* avCodecCtx,
    MediaSourceConfig const& config,
    int streamIndex,
    VideoEncodingProperties encodingProperties,
    HardwareDecoderStatus hardwareDecoderStatus)
    : H264AVCSampleProvider(reader, avFormatCtx, avCodecCtx, config, streamIndex, encodingProperties, hardwareDecoderStatus)
{
}

HEVCSampleProvider::~HEVCSampleProvider()
{
}

HRESULT HEVCSampleProvider::GetSPSAndPPSBuffer(DataWriter const& dataWriter, BYTE* buf, UINT32 length)
{
    HRESULT hr = S_OK;

    // Get the position of the SPS
    if (buf == nullptr || length < 4)
    {
        // The data isn't present
        hr = E_FAIL;
    }
    if (SUCCEEDED(hr))
    {
        if (length > 22 && (buf[0] || buf[1] || buf[2] > 1)) {
            /* Extradata is in hvcC format */
            int i, j, num_arrays;
            int pos = 21;

            m_nalLenSize = (buf[pos++] & 3) + 1;
            num_arrays = buf[pos++];

            /* Decode nal units from hvcC. */
            for (i = 0; i < num_arrays; i++) {
                int type = buf[pos++] & 0x3f;
                int cnt = ReadMultiByteValue(buf, pos, 2);
                pos += 2;

                for (j = 0; j < cnt; j++) {
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
            }
        }
        else
        {
            /* The stream and extradata contains raw NAL packets. No decoding needed. */
            auto extra = winrt::array_view(buf, length);
            dataWriter.WriteBytes(extra);
        }
    }

    return hr;
}
