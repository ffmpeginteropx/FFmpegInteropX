#pragma once
#include "FFmpegInteropXTranscoderProgress.g.h"

namespace winrt::FFmpegInteropX::implementation
{
    struct FFmpegInteropXTranscoderProgress : FFmpegInteropXTranscoderProgressT<FFmpegInteropXTranscoderProgress>
    {
        FFmpegInteropXTranscoderProgress() = default;

        int32_t PercentComplete();
    };
}
