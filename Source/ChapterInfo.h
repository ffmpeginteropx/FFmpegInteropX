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
        ChapterInfo(hstring const& codecName, hstring const& language, int32_t const& index, hstring const& imageType);
        hstring Title();
        hstring CodecName();
        hstring Language();
        int32_t Index();
        hstring ImageType();
        Windows::Foundation::TimeSpan StartTime();
        Windows::Foundation::TimeSpan Duration();

    private:
        hstring codecName{};
        hstring language{};
        int32_t index{};
        hstring imageType{};
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
