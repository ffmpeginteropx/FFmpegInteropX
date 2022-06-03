#pragma once
#include "SubtitleStreamInfo.g.h"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropXWinUI::implementation
{
    struct SubtitleStreamInfo : SubtitleStreamInfoT<SubtitleStreamInfo>
    {
        SubtitleStreamInfo() = default;

        SubtitleStreamInfo(hstring const& name, hstring const& language, hstring const& codecName, FFmpegInteropXWinUI::StreamDisposition const& disposition, bool isDefault, bool isForced, Windows::Media::Core::TimedMetadataTrack const& track, bool isExternal);
        bool IsExternal();
        bool IsForced();
        Windows::Media::Core::TimedMetadataTrack SubtitleTrack();
        hstring Name();
        hstring Language();
        hstring CodecName();
        FFmpegInteropXWinUI::StreamDisposition Disposition();
        int64_t Bitrate();
        bool IsDefault();
    };
}
namespace winrt::FFmpegInteropXWinUI::factory_implementation
{
    struct SubtitleStreamInfo : SubtitleStreamInfoT<SubtitleStreamInfo, implementation::SubtitleStreamInfo>
    {
    };
}
