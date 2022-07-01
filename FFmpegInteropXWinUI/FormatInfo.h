#pragma once
#include "FormatInfo.g.h"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropXWinUI::implementation
{
    struct FormatInfo : FormatInfoT<FormatInfo>
    {
        FormatInfo() = default;

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
namespace winrt::FFmpegInteropXWinUI::factory_implementation
{
    struct FormatInfo : FormatInfoT<FormatInfo, implementation::FormatInfo>
    {
    };
}
