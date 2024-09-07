#pragma once
#include "SubtitleStreamInfo.g.h"

namespace winrt::FFmpegInteropX::implementation
{
    struct SubtitleStreamInfo : SubtitleStreamInfoT<SubtitleStreamInfo>
    {
        SubtitleStreamInfo(hstring const& name, hstring const& language, hstring const& codecName, FFmpegInteropX::StreamDisposition const& disposition, bool isDefault, bool isForced, Windows::Media::Core::TimedMetadataTrack const& track, bool isExternal, int32_t streamIndex);
        bool IsExternal();
        bool IsForced();
        Windows::Media::Core::TimedMetadataTrack SubtitleTrack();
        hstring Name();
        hstring Language();
        hstring CodecName();
        FFmpegInteropX::StreamDisposition Disposition();
        int64_t Bitrate();
        bool IsDefault();
        int32_t StreamIndex();

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
        int streamIndex = -1;
    };
}
namespace winrt::FFmpegInteropX::factory_implementation
{
    struct SubtitleStreamInfo : SubtitleStreamInfoT<SubtitleStreamInfo, implementation::SubtitleStreamInfo>
    {
    };
}
