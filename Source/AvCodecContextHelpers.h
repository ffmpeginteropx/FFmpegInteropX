#pragma once

#include "pch.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/channel_layout.h>
}


class AvCodecContextHelpers
{
public:
    static int GetNBChannels(AVCodecContext* avCodecCtx)
    {
        return avCodecCtx->profile == FF_PROFILE_AAC_HE_V2 && avCodecCtx->ch_layout.nb_channels == 1 ? 2 : avCodecCtx->ch_layout.nb_channels;
    }

    static UINT64 GetChannelLayout(AVCodecContext* avCodecCtx)
    {
        if (avCodecCtx->profile == FF_PROFILE_AAC_HE_V2 && avCodecCtx->ch_layout.nb_channels == 1)
        {
            return AV_CH_LAYOUT_STEREO;
        }
        else
        {
            return
                avCodecCtx->ch_layout.order == AVChannelOrder::AV_CHANNEL_ORDER_NATIVE && avCodecCtx->ch_layout.u.mask ?
                    avCodecCtx->ch_layout.u.mask :
                    GetDefaultChannelLayout(avCodecCtx->ch_layout.nb_channels);
        }
    }

    static UINT64 GetDefaultChannelLayout(int channels)
    {
        if (channels == 6)
        {
            return AV_CH_LAYOUT_5POINT1;
        }
        else
        {
            AVChannelLayout layout;
            av_channel_layout_default(&layout, channels);
            return layout.u.mask;
        }
    }

private:
    AvCodecContextHelpers() {}
};




