#pragma once
#include "SubtitleParser.g.h"

namespace winrt::FFmpegInteropX::implementation
{
    struct SubtitleParser : SubtitleParserT<SubtitleParser>
    {
        SubtitleParser() = default;

        static winrt::Windows::Foundation::IAsyncOperation<winrt::FFmpegInteropX::SubtitleParser> AddExternalSubtitleAsync(winrt::Windows::Storage::Streams::IRandomAccessStream stream, hstring streamName);
        static winrt::Windows::Foundation::IAsyncOperation<winrt::FFmpegInteropX::SubtitleParser> AddExternalSubtitleAsync(winrt::Windows::Storage::Streams::IRandomAccessStream stream);
        winrt::FFmpegInteropX::SubtitleStreamInfo SubtitleTrack();
        winrt::Windows::Foundation::TimeSpan GetStreamDelay();
        void SetStreamDelay(winrt::Windows::Foundation::TimeSpan const& delay);
        void Close();

    private:
        winrt::com_ptr<FFmpegMediaSource> mediaSource = { nullptr };
        TimedMetadataTrack track = { nullptr };
        winrt::com_ptr<SubtitleStreamInfo> streamInfo = { nullptr };
    };
}
namespace winrt::FFmpegInteropX::factory_implementation
{
    struct SubtitleParser : SubtitleParserT<SubtitleParser, implementation::SubtitleParser>
    {
    };
}
