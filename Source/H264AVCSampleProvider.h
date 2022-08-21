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
#include "NALPacketSampleProvider.h"


class H264AVCSampleProvider :
    public NALPacketSampleProvider
{
public:
    virtual ~H264AVCSampleProvider();

    H264AVCSampleProvider(
        std::shared_ptr<FFmpegReader> reader,
        AVFormatContext* avFormatCtx,
        AVCodecContext* avCodecCtx,
        MediaSourceConfig const& config,
        int streamIndex,
        VideoEncodingProperties encodingProperties,
        HardwareDecoderStatus hardwareDecoderStatus);
    virtual HRESULT GetSPSAndPPSBuffer(DataWriter const& dataWriter, BYTE* buf, UINT32 length) override;
    virtual HRESULT WriteNALPacket(AVPacket* avPacket, IBuffer* pBuffer) override;
    virtual HRESULT WriteNALPacketAfterExtradata(AVPacket* avPacket, DataWriter const& dataWriter) override;
    int ReadMultiByteValue(BYTE* buffer, int position, int numBytes);
    int m_nalLenSize = 0;
};
