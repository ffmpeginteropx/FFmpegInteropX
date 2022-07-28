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
        virtual ~AvSubtitleContextTrack();
        AvSubtitleContextTrack();
        const AVCodec* m_avSubtitleCodec;
        AVCodecContext* avSubtitleCodecCtx;
        int index;
    private:
    };
}
