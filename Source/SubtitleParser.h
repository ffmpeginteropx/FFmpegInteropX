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

        ///<summary>Adds an external subtitle from a stream.</summary>
        ///<param name="stream">The subtitle stream.</param>
        ///<param name="streamName">The name to use for the subtitle.</param>
        ///<param name="config">Used to configure subtitles</param>
        ///<param name="videoDescriptor">Descriptor of the video stream that the subtitle will be attached to. Used to compute sizes of image subs.</param>
        static IAsyncOperation<winrt::FFmpegInteropX::SubtitleParser> ReadSubtitleAsync(winrt::Windows::Storage::Streams::IRandomAccessStream stream, hstring streamName, winrt::FFmpegInteropX::MediaSourceConfig config, winrt::Windows::Media::Core::VideoStreamDescriptor videoDescriptor);

        ///<summary>Adds an external subtitle from a stream.</summary>
        ///<param name="stream">The subtitle stream.</param>
        static IAsyncOperation<winrt::FFmpegInteropX::SubtitleParser> ReadSubtitleAsync(winrt::Windows::Storage::Streams::IRandomAccessStream stream);

        ///<summary>Adds an external subtitle from an URI.</summary>
        ///<param name="uri">The subtitle URI.</param>
        ///<param name="streamName">The name to use for the subtitle.</param>
        ///<param name="config">Used to configure subtitles</param>
        ///<param name="videoDescriptor">Descriptor of the video stream that the subtitle will be attached to. Used to compute sizes of image subs.</param>
        static IAsyncOperation<winrt::FFmpegInteropX::SubtitleParser> ReadSubtitleAsync(Uri uri, hstring streamName, winrt::FFmpegInteropX::MediaSourceConfig config, winrt::Windows::Media::Core::VideoStreamDescriptor videoDescriptor);

        ///<summary>Adds an external subtitle from an URI.</summary>
        ///<param name="stream">The subtitle URI.</param>
        static IAsyncOperation<winrt::FFmpegInteropX::SubtitleParser> ReadSubtitleAsync(Uri uri);

        ///<summary>Returns the parsed subtitle track.</summary>
        winrt::FFmpegInteropX::SubtitleStreamInfo SubtitleTrack();

        ///<summary>Gets the presentation timestamp delay for the subtitle stream. </summary>
        TimeSpan GetStreamDelay();

        ///<summary>Sets a presentation timestamp delay for the subtitle stream. Audio, video and subtitle synchronisation can be achieved this way. A positive value will cause samples (or subtitles) to be rendered at a later time. A negative value will make rendering come sooner</summary>
        void SetStreamDelay(TimeSpan const& delay);

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
