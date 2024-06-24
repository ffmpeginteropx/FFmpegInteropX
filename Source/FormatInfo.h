#pragma once
#include "FormatInfo.g.h"

namespace winrt::FFmpegInteropX::implementation
{
    struct FormatInfo : FormatInfoT<FormatInfo>
    {
        FormatInfo(hstring const& title, hstring const& formatName, Windows::Foundation::TimeSpan const& duration, int64_t bitrate);
        hstring Title();
        hstring FormatName();
        Windows::Foundation::TimeSpan Duration();
        int64_t Bitrate();

    private:
        winrt::hstring title{};
        winrt::hstring formatName{};
        TimeSpan duration{};
        int64_t bitrate = 0;
    };
}
namespace winrt::FFmpegInteropX::factory_implementation
{
    struct FormatInfo : FormatInfoT<FormatInfo, implementation::FormatInfo>
    {
    };
}
