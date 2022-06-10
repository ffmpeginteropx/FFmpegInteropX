#pragma once
#include "FFmpegMediaSource.g.h"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropXWinUI::implementation
{
    struct FFmpegMediaSource : FFmpegMediaSourceT<FFmpegMediaSource>
    {
        FFmpegMediaSource() = default;

        static Windows::Foundation::IAsyncOperation<FFmpegInteropXWinUI::FFmpegMediaSource> CreateFromStreamAsync(Windows::Storage::Streams::IRandomAccessStream stream, FFmpegInteropXWinUI::MediaSourceConfig config);
        static Windows::Foundation::IAsyncOperation<FFmpegInteropXWinUI::FFmpegMediaSource> CreateFromStreamAsync(Windows::Storage::Streams::IRandomAccessStream stream);
        static Windows::Foundation::IAsyncOperation<FFmpegInteropXWinUI::FFmpegMediaSource> CreateFromUriAsync(hstring uri, FFmpegInteropXWinUI::MediaSourceConfig config);
        static Windows::Foundation::IAsyncOperation<FFmpegInteropXWinUI::FFmpegMediaSource> CreateFromUriAsync(hstring uri);
        void SetSubtitleDelay(Windows::Foundation::TimeSpan const& delay);
        void SetFFmpegAudioFilters(hstring const& audioFilters);
        void SetFFmpegVideoFilters(hstring const& videoEffects);
        void DisableAudioEffects();
        void DisableVideoEffects();
        FFmpegInteropXWinUI::MediaThumbnailData ExtractThumbnail();
        Windows::Media::Core::MediaStreamSource GetMediaStreamSource();
        Windows::Media::Playback::MediaPlaybackItem CreateMediaPlaybackItem();
        Windows::Media::Playback::MediaPlaybackItem CreateMediaPlaybackItem(Windows::Foundation::TimeSpan const& startTime);
        Windows::Media::Playback::MediaPlaybackItem CreateMediaPlaybackItem(Windows::Foundation::TimeSpan const& startTime, Windows::Foundation::TimeSpan const& durationLimit);
        Windows::Foundation::IAsyncOperation<Windows::Foundation::Collections::IVectorView<FFmpegInteropXWinUI::SubtitleStreamInfo>> AddExternalSubtitleAsync(Windows::Storage::Streams::IRandomAccessStream stream, hstring streamName);
        Windows::Foundation::IAsyncOperation<Windows::Foundation::Collections::IVectorView<FFmpegInteropXWinUI::SubtitleStreamInfo>> AddExternalSubtitleAsync(Windows::Storage::Streams::IRandomAccessStream stream);
        FFmpegInteropXWinUI::MediaSourceConfig Configuration();
        Windows::Foundation::Collections::IVectorView<Windows::Foundation::Collections::IKeyValuePair<hstring, hstring>> MetadataTags();
        Windows::Foundation::TimeSpan Duration();
        FFmpegInteropXWinUI::VideoStreamInfo CurrentVideoStream();
        FFmpegInteropXWinUI::AudioStreamInfo CurrentAudioStream();
        Windows::Foundation::Collections::IVectorView<FFmpegInteropXWinUI::VideoStreamInfo> VideoStreams();
        Windows::Foundation::Collections::IVectorView<FFmpegInteropXWinUI::AudioStreamInfo> AudioStreams();
        Windows::Foundation::Collections::IVectorView<FFmpegInteropXWinUI::SubtitleStreamInfo> SubtitleStreams();
        Windows::Foundation::Collections::IVectorView<FFmpegInteropXWinUI::ChapterInfo> ChapterInfos();
        FFmpegInteropXWinUI::FormatInfo FormatInfo();
        bool HasThumbnail();
        Windows::Media::Playback::MediaPlaybackItem PlaybackItem();
        Windows::Foundation::TimeSpan SubtitleDelay();
        Windows::Foundation::TimeSpan BufferTime();
        Windows::Media::Playback::MediaPlaybackSession PlaybackSession();
        void PlaybackSession(Windows::Media::Playback::MediaPlaybackSession const& value);
        void Close();
    };
}
namespace winrt::FFmpegInteropXWinUI::factory_implementation
{
    struct FFmpegMediaSource : FFmpegMediaSourceT<FFmpegMediaSource, implementation::FFmpegMediaSource>
    {
    };
}
