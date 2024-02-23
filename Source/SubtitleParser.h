#pragma once
#include "SubtitleParser.g.h"
#include <FFmpegMediaSource.h>

namespace winrt::FFmpegInteropX::implementation
{
    struct SubtitleParser : SubtitleParserT<SubtitleParser>
    {
        SubtitleParser() = default;

        SubtitleParser(winrt::com_ptr<winrt::FFmpegInteropX::implementation::FFmpegMediaSource> interopMSS)
        {
            this->interopMSS = interopMSS;
        }

        static winrt::Windows::Foundation::IAsyncOperation<winrt::FFmpegInteropX::SubtitleParser> ReadSubtitleAsync(winrt::Windows::Storage::Streams::IRandomAccessStream stream, hstring streamName, winrt::FFmpegInteropX::MediaSourceConfig config, winrt::Windows::Media::Core::VideoStreamDescriptor videoDescriptor, uint64_t windowId, bool useHdr);
        static winrt::Windows::Foundation::IAsyncOperation<winrt::FFmpegInteropX::SubtitleParser> ReadSubtitleAsync(winrt::Windows::Storage::Streams::IRandomAccessStream stream);
        winrt::FFmpegInteropX::SubtitleStreamInfo SubtitleTrack();
        winrt::Windows::Foundation::TimeSpan GetStreamDelay();
        void SetStreamDelay(winrt::Windows::Foundation::TimeSpan const& delay);
        void Close();

    private:
        winrt::com_ptr<FFmpegMediaSource> interopMSS = { nullptr };
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
