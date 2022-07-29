#include "pch.h"
#include "AvSubtitleEventArgs.h"
#include "AvSubtitleEventArgs.g.cpp"

namespace winrt::FFmpegInteropX::implementation
{
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
    int32_t AvSubtitleEventArgs::StartTime()
    {
        return starttime;
    }
    int32_t AvSubtitleEventArgs::Duration()
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
