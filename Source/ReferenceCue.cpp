#include "pch.h"
#include "ReferenceCue.h"

ReferenceCue::ReferenceCue(winrt::Windows::Media::Core::IMediaCue const& addedCue, winrt::Windows::Media::Core::IMediaCue const& changedCue)
{
    this->addedCue = addedCue;
    this->duration = addedCue.Duration();
    this->id = addedCue.Id();
    this->startTime = addedCue.StartTime();
    this->changedCue = changedCue;
}

ReferenceCue::ReferenceCue(winrt::Windows::Foundation::TimeSpan const& startTime, winrt::Windows::Foundation::TimeSpan const& duration, winrt::Windows::Media::Core::IMediaCue const& changedCue)
{
    this->startTime = startTime;
    this->duration = duration;
    this->changedCue = changedCue;
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
winrt::Windows::Media::Core::IMediaCue ReferenceCue::AddedCue()
{
    return addedCue;
}
winrt::Windows::Media::Core::IMediaCue ReferenceCue::ChangedCue()
{
    return changedCue;
}
