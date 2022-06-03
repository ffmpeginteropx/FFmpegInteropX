#include "pch.h"
#include "ChapterInfo.h"
#include "ChapterInfo.g.cpp"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropXWinUI::implementation
{
    ChapterInfo::ChapterInfo(hstring const& title, Windows::Foundation::TimeSpan const& startTime, Windows::Foundation::TimeSpan const& duration)
    {
        throw hresult_not_implemented();
    }
    hstring ChapterInfo::Title()
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::TimeSpan ChapterInfo::StartTime()
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::TimeSpan ChapterInfo::Duration()
    {
        throw hresult_not_implemented();
    }
}
