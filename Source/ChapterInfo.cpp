#include "pch.h"
#include "ChapterInfo.h"
#include "ChapterInfo.g.cpp"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropX::implementation
{
    ChapterInfo::ChapterInfo(hstring const& title, Windows::Foundation::TimeSpan const& startTime, Windows::Foundation::TimeSpan const& duration)
    {
        this->title = title;
        this->startTime = startTime;
        this->duration = duration;
    }

    ChapterInfo::ChapterInfo(hstring const& codecName, hstring const& language, int32_t const& index, hstring const& imageType)
    {
        this->codecName = codecName;
        this->language = language;
        this->index = index;
        this->imageType = imageType;
    }

    hstring ChapterInfo::CodecName()
    {
        return this->codecName;
    }
    hstring ChapterInfo::Language()
    {
        return this->language;
    }
    int32_t ChapterInfo::Index()
    {
        return this->index;
    }
    hstring ChapterInfo::ImageType()
    {
        return this->imageType;
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
