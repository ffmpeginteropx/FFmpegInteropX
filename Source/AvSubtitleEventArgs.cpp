#include "pch.h"
#include "AvSubtitleEventArgs.h"
#include "AvSubtitleEventArgs.g.cpp"

namespace winrt::FFmpegInteropX::implementation
{
    AvSubtitleEventArgs::AvSubtitleEventArgs(Windows::Foundation::TimeSpan const& startTime, Windows::Foundation::TimeSpan const& duration)
    {
        this->starttime = startTime;
        this->duration = duration;
    }
    AvSubtitleEventArgs::AvSubtitleEventArgs()
    {
    }
    hstring AvSubtitleEventArgs::Type()
    {
        return type;
    }
    int32_t AvSubtitleEventArgs::Index()
    {
        return index;
    }
    Windows::Foundation::TimeSpan AvSubtitleEventArgs::StartTime()
    {
        return starttime;
    }
    Windows::Foundation::TimeSpan AvSubtitleEventArgs::Duration()
    {
        return duration;
    }
    int32_t AvSubtitleEventArgs::Width()
    {
        return width;
    }
    int32_t AvSubtitleEventArgs::Height()
    {
        return height;
    }
    winrt::Windows::Storage::Streams::IBuffer AvSubtitleEventArgs::Buffer()
    {
        return buffer;
    }
    winrt::Windows::Storage::Streams::IBuffer AvSubtitleEventArgs::Buffer2()
    {
        return buffer2;
    }
}
