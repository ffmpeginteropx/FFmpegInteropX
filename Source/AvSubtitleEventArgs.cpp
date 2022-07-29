#include "pch.h"
#include "AvSubtitleEventArgs.h"
#include "AvSubtitleEventArgs.g.cpp"

namespace winrt::FFmpegInteropX::implementation
{
    AvSubtitleEventArgs::AvSubtitleEventArgs(Windows::Foundation::TimeSpan const& startTime, Windows::Foundation::TimeSpan const& duration, winrt::Windows::Storage::Streams::IBuffer const& buffer, winrt::Windows::Storage::Streams::IBuffer const& buffer2, int32_t const& index, int32_t const& width, int32_t const& height, hstring const& type)
    {
        this->starttime = startTime;
        this->duration = duration;
        this->buffer = buffer;
        this->buffer2 = buffer2;
        this->width = width;
        this->height = height;
        this->type = type;
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
