#pragma once
#include "ChapterInfo.g.h"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropX::implementation
{
    struct ChapterInfo : ChapterInfoT<ChapterInfo>
    {
        ChapterInfo(hstring const& title, Windows::Foundation::TimeSpan const& startTime, Windows::Foundation::TimeSpan const& duration);
        hstring Title();
        Windows::Foundation::TimeSpan StartTime();
        Windows::Foundation::TimeSpan Duration();

    private:
        hstring title{};
        TimeSpan startTime{};
        TimeSpan duration{};
    };
}
namespace winrt::FFmpegInteropX::factory_implementation
{
    struct ChapterInfo : ChapterInfoT<ChapterInfo, implementation::ChapterInfo>
    {
    };
}
