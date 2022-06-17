#include "pch.h"
#include "FormatInfo.h"
#include "FormatInfo.g.cpp"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropXWinUI::implementation
{
    FormatInfo::FormatInfo(hstring const& title, hstring const& formatName, Windows::Foundation::TimeSpan const& duration, int64_t bitrate)
    {
        this->title = title;
        this->formatName = formatName;
        this->duration = duration;
        this->bitrate = bitrate;
    }

    hstring FormatInfo::Title()
    {
        return title;
    }

    hstring FormatInfo::FormatName()
    {
        return formatName;
    }

    Windows::Foundation::TimeSpan FormatInfo::Duration()
    {
        return duration;
    }

    int64_t FormatInfo::Bitrate()
    {
        return bitrate;
    }
}
