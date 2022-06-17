#include "pch.h"
#include "ReferenceCue.h"
#include "ReferenceCue.g.cpp"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropXWinUI::implementation
{
    ReferenceCue::ReferenceCue(Windows::Media::Core::IMediaCue const& other)
    {
        this->cueRef = other;
        this->duration = other.Duration();
        this->id = other.Id();
        this->startTime = other.StartTime();
    }

    void ReferenceCue::StartTime(Windows::Foundation::TimeSpan const& value)
    {
        this->startTime = value;
    }
    Windows::Foundation::TimeSpan ReferenceCue::StartTime()
    {
        return startTime;
    }
    void ReferenceCue::Duration(Windows::Foundation::TimeSpan const& value)
    {
        this->duration = value;
    }
    Windows::Foundation::TimeSpan ReferenceCue::Duration()
    {
        return duration;
    }
    void ReferenceCue::Id(hstring const& value)
    {
        id = value;
    }
    hstring ReferenceCue::Id()
    {
        return id;
    }
}
