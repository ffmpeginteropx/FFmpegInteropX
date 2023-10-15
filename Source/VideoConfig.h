#pragma once
#include "VideoConfig.g.h"
#include "ConfigurationCommon.h"

namespace winrt::FFmpegInteropX::implementation
{
    struct VideoConfig : VideoConfigT<VideoConfig>
    {
        VideoConfig() = default;

        ///<summary>Sets the video decoder mode. Default is Automatic.</summary>
        PROPERTY(VideoDecoderMode, FFmpegInteropX::VideoDecoderMode, VideoDecoderMode::Automatic);

        ///<summary>Sets the HDR color support mode. Default is Automatic.</summary>
        PROPERTY(HdrSupport, FFmpegInteropX::HdrSupport, HdrSupport::Automatic);

        ///<summary>Max profile allowed for H264 system decoder. Default: High Profile (100). See FF_PROFILE_H264_* values.</summary>
        PROPERTY(SystemDecoderH264MaxProfile, int32_t, FF_PROFILE_H264_HIGH);

        ///<summary>Max level allowed for H264 system decoder. Default: Level 4.1 (41). Use -1 to disable level check.</summary>
        ///<remarks>Most H264 HW decoders only support Level 4.1, so this is the default.</remarks>
        PROPERTY(SystemDecoderH264MaxLevel, int32_t, 41);


        ///<summary>Max profile allowed for HEVC system decoder. Default: High10 Profile (2). See FF_PROFILE_HEVC_* values.</summary>
        PROPERTY(SystemDecoderHEVCMaxProfile, int32_t, FF_PROFILE_HEVC_MAIN_10);

        ///<summary>Max level allowed for HEVC system decoder. Default: Disabled (-1).</summary>
        ///<remarks>Encoded as: 30*Major + 3*Minor. So Level 6.0 = 30*6 = 180, 5.1 = 30*5 + 3*1 = 163, 4.1 = 123.
        ///Many HEVC HW decoders support even very high levels, so we disable the check by default.</remarks>
        PROPERTY(SystemDecoderHEVCMaxLevel, int32_t, -1);

        ///<summary>Allow video output in IYuv format.</summary>
        PROPERTY(VideoOutputAllowIyuv, bool, false);

        ///<summary>Allow video output in 10bit formats.</summary>
        PROPERTY(VideoOutputAllow10bit, bool, true);

        ///<summary>Allow video output in BGRA format - required for video transparency.</summary>
        PROPERTY(VideoOutputAllowBgra8, bool, false);

        ///<summary>Allow video output in NV12 format.</summary>
        PROPERTY(VideoOutputAllowNv12, bool, true);

        ///<summary>The maximum number of video decoding threads. Setting to means using the number of logical CPU cores.</summary>
        PROPERTY(MaxVideoThreads, int32_t, 0);

        ///<remarks>Using FFmpeg video filters will degrade playback performance, since they run on the CPU and not on the GPU.</remarks>
        PROPERTY_CONST(FFmpegVideoFilters, hstring, {});

    };
}
