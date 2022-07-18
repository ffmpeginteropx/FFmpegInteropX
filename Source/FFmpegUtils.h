#pragma once
#include "pch.h"
class FFmpegUtils
{
public:
    static AVPixelFormat get_format(struct AVCodecContext* s, const enum AVPixelFormat* fmt)
    {
        AVPixelFormat result_sw = (AVPixelFormat)-1;
        AVPixelFormat result_hw = (AVPixelFormat)-1;
        AVPixelFormat format;
        int index = 0;
        do
        {
            format = fmt[index++];

            //		
            if (format != -1)
            {
                if (s->hw_device_ctx && format == AV_PIX_FMT_D3D11)
                {
                    // we only support D3D11 HW format (not D3D11_VLD)
                    result_hw = format;
                }
                else if (result_sw == -1 && !is_hwaccel_pix_fmt(format))
                {
                    // take first non hw accelerated format
                    result_sw = format;
                }
                else if (format == AV_PIX_FMT_NV12 && result_sw != AV_PIX_FMT_YUVA420P)
                {
                    // switch SW format to NV12 if available, unless this is an alpha channel file
                    result_sw = format;
                }
            }
        } while (format != -1);


        if (result_hw != -1)
        {
            return result_hw;
        }
        else
        {
            return result_sw;
        }
    }

    static int is_hwaccel_pix_fmt(enum AVPixelFormat pix_fmt)
    {
        const AVPixFmtDescriptor* desc = av_pix_fmt_desc_get(pix_fmt);
        return desc->flags & AV_PIX_FMT_FLAG_HWACCEL;
    }
};

