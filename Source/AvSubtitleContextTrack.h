//*****************************************************************************
//
//	Copyright 2015 Microsoft Corporation
//
//	Licensed under the Apache License, Version 2.0 (the "License");
//	you may not use this file except in compliance with the License.
//	You may obtain a copy of the License at
#pragma once

namespace FFmpegInteropX
{
    class AvSubtitleContextTrack
    {
    public:
        virtual ~AvSubtitleContextTrack()
        {
            //if (this->avSubtitleCodecCtx != nullptr && this->avSubtitleCodecCtx)
            //{
            //    auto t = &this->avSubtitleCodecCtx;
            //    avcodec_free_context(const_cast <AVCodecContext**>(t));
            //    DebugMessage(L"AvSubtitleContextTrack reader destroyed1\n");
            //}
            DebugMessage(L"AvSubtitleContextTrack reader destroyed2\n");
        }
        AvSubtitleContextTrack()
        {

        }
        const AVCodec* m_avSubtitleCodec;
        AVCodecContext* avSubtitleCodecCtx;
        int index;
    private:
    };
}
