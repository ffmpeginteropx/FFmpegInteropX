#pragma once
#include "ReferenceCue.g.h"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropXWinUI::implementation
{
    struct ReferenceCue : ReferenceCueT<ReferenceCue>
    {
        ReferenceCue() = default;

        ReferenceCue(Windows::Media::Core::IMediaCue const& other);
        void StartTime(Windows::Foundation::TimeSpan const& value);
        Windows::Foundation::TimeSpan StartTime();
        void Duration(Windows::Foundation::TimeSpan const& value);
        Windows::Foundation::TimeSpan Duration();
        void Id(hstring const& value);
        hstring Id();
    };
}
namespace winrt::FFmpegInteropXWinUI::factory_implementation
{
    struct ReferenceCue : ReferenceCueT<ReferenceCue, implementation::ReferenceCue>
    {
    };
}
