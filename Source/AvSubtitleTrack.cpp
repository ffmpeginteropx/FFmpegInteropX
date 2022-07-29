#include "pch.h"
#include "AvSubtitleTrack.h"
#include "AvSubtitleTrack.g.cpp"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropX::implementation
{
    AvSubtitleTrack::AvSubtitleTrack(hstring const& codecName, hstring const& language, int32_t const& index, hstring const& imageType)
    {
        this->codecName = codecName;
        this->language = language;
        this->index = index;
        this->imageType = imageType;
    }

    hstring AvSubtitleTrack::CodecName()
    {
        return this->codecName;
    }
    hstring AvSubtitleTrack::Language()
    {
        return this->language;
    }
    int32_t AvSubtitleTrack::Index()
    {
        return this->index;
    }
    hstring AvSubtitleTrack::ImageType()
    {
        return this->imageType;
    }
}
