#include "pch.h"
#include "ChapterInfo.h"
#include "ChapterInfo.g.cpp"

namespace winrt::FFmpegInteropX::implementation
{
    ChapterInfo::ChapterInfo(hstring const& title, Windows::Foundation::TimeSpan const& startTime, Windows::Foundation::TimeSpan const& duration)
    {
        this->title = title;
        this->startTime = startTime;
        this->duration = duration;
    }

    hstring ChapterInfo::Title()
    {
        return this->title;
    }

    Windows::Foundation::TimeSpan ChapterInfo::StartTime()
    {
        return this->startTime;
    }

    Windows::Foundation::TimeSpan ChapterInfo::Duration()
    {
        return this->duration;
    }
}
