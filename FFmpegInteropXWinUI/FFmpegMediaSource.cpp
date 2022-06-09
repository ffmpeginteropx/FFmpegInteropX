#include "pch.h"
#include "FFmpegMediaSource.h"
#include "FFmpegMediaSource.g.cpp"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropXWinUI::implementation
{
    Windows::Foundation::IAsyncOperation<FFmpegInteropXWinUI::FFmpegMediaSource> FFmpegMediaSource::CreateFromStreamAsync(Windows::Storage::Streams::IRandomAccessStream stream, FFmpegInteropXWinUI::MediaSourceConfig config)
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::IAsyncOperation<FFmpegInteropXWinUI::FFmpegMediaSource> FFmpegMediaSource::CreateFromStreamAsync(Windows::Storage::Streams::IRandomAccessStream stream)
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::IAsyncOperation<FFmpegInteropXWinUI::FFmpegMediaSource> FFmpegMediaSource::CreateFromUriAsync(hstring uri, FFmpegInteropXWinUI::MediaSourceConfig config)
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::IAsyncOperation<FFmpegInteropXWinUI::FFmpegMediaSource> FFmpegMediaSource::CreateFromUriAsync(hstring uri)
    {
        throw hresult_not_implemented();
    }
    void FFmpegMediaSource::SetSubtitleDelay(Windows::Foundation::TimeSpan const& delay)
    {
        throw hresult_not_implemented();
    }
    void FFmpegMediaSource::SetFFmpegAudioFilters(hstring const& audioFilters)
    {
        throw hresult_not_implemented();
    }
    void FFmpegMediaSource::SetFFmpegVideoFilters(hstring const& videoEffects)
    {
        throw hresult_not_implemented();
    }
    void FFmpegMediaSource::DisableAudioEffects()
    {
        throw hresult_not_implemented();
    }
    void FFmpegMediaSource::DisableVideoEffects()
    {
        throw hresult_not_implemented();
    }
    FFmpegInteropXWinUI::MediaThumbnailData FFmpegMediaSource::ExtractThumbnail()
    {
        throw hresult_not_implemented();
    }
    Windows::Media::Core::MediaStreamSource FFmpegMediaSource::GetMediaStreamSource()
    {
        throw hresult_not_implemented();
    }
    Windows::Media::Playback::MediaPlaybackItem FFmpegMediaSource::CreateMediaPlaybackItem()
    {
        throw hresult_not_implemented();
    }
    Windows::Media::Playback::MediaPlaybackItem FFmpegMediaSource::CreateMediaPlaybackItem(Windows::Foundation::TimeSpan const& startTime)
    {
        throw hresult_not_implemented();
    }
    Windows::Media::Playback::MediaPlaybackItem FFmpegMediaSource::CreateMediaPlaybackItem(Windows::Foundation::TimeSpan const& startTime, Windows::Foundation::TimeSpan const& durationLimit)
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::IAsyncOperation<Windows::Foundation::Collections::IVectorView<FFmpegInteropXWinUI::SubtitleStreamInfo>> FFmpegMediaSource::AddExternalSubtitleAsync(Windows::Storage::Streams::IRandomAccessStream stream, hstring streamName)
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::IAsyncOperation<Windows::Foundation::Collections::IVectorView<FFmpegInteropXWinUI::SubtitleStreamInfo>> FFmpegMediaSource::AddExternalSubtitleAsync(Windows::Storage::Streams::IRandomAccessStream stream)
    {
        throw hresult_not_implemented();
    }
    FFmpegInteropXWinUI::MediaSourceConfig FFmpegMediaSource::Configuration()
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::Collections::IVectorView<Windows::Foundation::Collections::IKeyValuePair<hstring, hstring>> FFmpegMediaSource::MetadataTags()
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::TimeSpan FFmpegMediaSource::Duration()
    {
        throw hresult_not_implemented();
    }
    FFmpegInteropXWinUI::VideoStreamInfo FFmpegMediaSource::CurrentVideoStream()
    {
        throw hresult_not_implemented();
    }
    FFmpegInteropXWinUI::AudioStreamInfo FFmpegMediaSource::CurrentAudioStream()
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::Collections::IVectorView<FFmpegInteropXWinUI::VideoStreamInfo> FFmpegMediaSource::VideoStreams()
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::Collections::IVectorView<FFmpegInteropXWinUI::AudioStreamInfo> FFmpegMediaSource::AudioStreams()
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::Collections::IVectorView<FFmpegInteropXWinUI::SubtitleStreamInfo> FFmpegMediaSource::SubtitleStreams()
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::Collections::IVectorView<FFmpegInteropXWinUI::ChapterInfo> FFmpegMediaSource::ChapterInfos()
    {
        throw hresult_not_implemented();
    }
    FFmpegInteropXWinUI::FormatInfo FFmpegMediaSource::FormatInfo()
    {
        throw hresult_not_implemented();
    }
    bool FFmpegMediaSource::HasThumbnail()
    {
        throw hresult_not_implemented();
    }
    Windows::Media::Playback::MediaPlaybackItem FFmpegMediaSource::PlaybackItem()
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::TimeSpan FFmpegMediaSource::SubtitleDelay()
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::TimeSpan FFmpegMediaSource::BufferTime()
    {
        throw hresult_not_implemented();
    }
    Windows::Media::Playback::MediaPlaybackSession FFmpegMediaSource::PlaybackSession()
    {
        throw hresult_not_implemented();
    }
    void FFmpegMediaSource::Close()
    {
        throw hresult_not_implemented();
    }
}
