#include "pch.h"
#include "ReferenceCue.h"

ReferenceCue::ReferenceCue(winrt::Windows::Media::Core::IMediaCue const& other)
{
    this->cueRef = other;
    this->duration = other.Duration();
    this->id = other.Id();
    this->startTime = other.StartTime();
}

void ReferenceCue::StartTime(winrt::Windows::Foundation::TimeSpan const& value)
{
    this->startTime = value;
}

winrt::Windows::Foundation::TimeSpan ReferenceCue::StartTime()
{
    return startTime;
}
void ReferenceCue::Duration(winrt::Windows::Foundation::TimeSpan const& value)
{
    this->duration = value;
}
winrt::Windows::Foundation::TimeSpan ReferenceCue::Duration()
{
    return duration;
}
void ReferenceCue::Id(winrt::hstring const& value)
{
    id = value;
}
winrt::hstring ReferenceCue::Id()
{
    return id;
}
winrt::Windows::Media::Core::IMediaCue ReferenceCue::CueRef()
{
    return cueRef;
}
