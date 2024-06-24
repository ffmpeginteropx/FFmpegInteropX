#include "pch.h"
#include "FormatInfo.h"
#include "FormatInfo.g.cpp"

namespace winrt::FFmpegInteropX::implementation
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
