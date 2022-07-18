#pragma once
#include "SubtitleStreamInfo.g.h"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropX::implementation
{
    struct SubtitleStreamInfo : SubtitleStreamInfoT<SubtitleStreamInfo>
    {
        SubtitleStreamInfo(hstring const& name, hstring const& language, hstring const& codecName, FFmpegInteropX::StreamDisposition const& disposition, bool isDefault, bool isForced, Windows::Media::Core::TimedMetadataTrack const& track, bool isExternal);
        bool IsExternal();
        bool IsForced();
        Windows::Media::Core::TimedMetadataTrack SubtitleTrack();
        hstring Name();
        hstring Language();
        hstring CodecName();
        FFmpegInteropX::StreamDisposition Disposition();
        int64_t Bitrate();
        bool IsDefault();

        hstring name{};
        hstring language{};
        hstring codecName{};
        StreamDisposition disposition;
        bool isDefault = false;
        bool isForced = false;
        Windows::Media::Core::TimedMetadataTrack track = { nullptr };
        bool isExternal = false;
        bool SetDefault()
        {
            isDefault = true;
            return isDefault;
        }
    };
}
namespace winrt::FFmpegInteropX::factory_implementation
{
    struct SubtitleStreamInfo : SubtitleStreamInfoT<SubtitleStreamInfo, implementation::SubtitleStreamInfo>
    {
    };
}
