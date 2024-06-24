#include "pch.h"
#include "SubtitleStreamInfo.h"
#include "SubtitleStreamInfo.g.cpp"

namespace winrt::FFmpegInteropX::implementation
{
    SubtitleStreamInfo::SubtitleStreamInfo(hstring const& name, hstring const& language, hstring const& codecName, FFmpegInteropX::StreamDisposition const& disposition, bool isDefault, bool isForced, Windows::Media::Core::TimedMetadataTrack const& track, bool isExternal, int32_t streamIndex)
    {
        this->name = name;
        this->language = language;
        this->codecName = codecName;
        this->disposition = disposition;
        this->isDefault = isDefault;
        this->isForced = isForced;
        this->track = track;
        this->isExternal = isExternal;
        this->streamIndex = streamIndex;
    }

    bool SubtitleStreamInfo::IsExternal()
    {
        return isExternal;
    }

    bool SubtitleStreamInfo::IsForced()
    {
        return isForced;
    }

    Windows::Media::Core::TimedMetadataTrack SubtitleStreamInfo::SubtitleTrack()
    {
        return track;
    }

    hstring SubtitleStreamInfo::Name()
    {
        return name;
    }

    hstring SubtitleStreamInfo::Language()
    {
        return language;
    }

    hstring SubtitleStreamInfo::CodecName()
    {
        return codecName;
    }

    FFmpegInteropX::StreamDisposition SubtitleStreamInfo::Disposition()
    {
        return disposition;
    }

    int64_t SubtitleStreamInfo::Bitrate()
    {
        return 0;
    }

    bool SubtitleStreamInfo::IsDefault()
    {
        return isDefault;
    }

    int32_t SubtitleStreamInfo::StreamIndex()
    {
        return streamIndex;
    }
}
