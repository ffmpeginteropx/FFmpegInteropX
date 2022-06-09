#include "pch.h"
#include "SubtitleStreamInfo.h"
#include "SubtitleStreamInfo.g.cpp"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropXWinUI::implementation
{
    SubtitleStreamInfo::SubtitleStreamInfo(hstring const& name, hstring const& language, hstring const& codecName, FFmpegInteropXWinUI::StreamDisposition const& disposition, bool isDefault, bool isForced, Windows::Media::Core::TimedMetadataTrack const& track, bool isExternal)
    {
        throw hresult_not_implemented();
    }
    bool SubtitleStreamInfo::IsExternal()
    {
        throw hresult_not_implemented();
    }
    bool SubtitleStreamInfo::IsForced()
    {
        throw hresult_not_implemented();
    }
    Windows::Media::Core::TimedMetadataTrack SubtitleStreamInfo::SubtitleTrack()
    {
        return Windows::Media::Core::TimedMetadataTrack{ nullptr };
    }
    hstring SubtitleStreamInfo::Name()
    {
        throw hresult_not_implemented();
    }
    hstring SubtitleStreamInfo::Language()
    {
        throw hresult_not_implemented();
    }
    hstring SubtitleStreamInfo::CodecName()
    {
        throw hresult_not_implemented();
    }
    FFmpegInteropXWinUI::StreamDisposition SubtitleStreamInfo::Disposition()
    {
        throw hresult_not_implemented();
    }
    int64_t SubtitleStreamInfo::Bitrate()
    {
        throw hresult_not_implemented();
    }
    bool SubtitleStreamInfo::IsDefault()
    {
        throw hresult_not_implemented();
    }
}
