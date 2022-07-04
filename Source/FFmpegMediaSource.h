#pragma once
#include "FFmpegMediaSource.g.h"
#include "pch.h"
#include "MediaSampleProvider.h"
#include "SubtitleProvider.h"
#include "AttachedFile.h"
#include "AttachedFileHelper.h"
#include "MediaSourceConfig.h"
#include "CodecChecker.h"
#include "MediaMetadata.h"
#include "AudioStreamInfo.h"
#include "VideoStreamInfo.h"
#include "SubtitleStreamInfo.h"
#include "ChapterInfo.h"
#include "FormatInfo.h"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropX::implementation
{
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Windows::Foundation::Collections;
    using namespace winrt::Windows::Media::Core;
    using namespace winrt::Windows::Media::Playback;
    using namespace winrt::Windows::Foundation::Collections;
    using namespace winrt::Windows::UI::Core;
    using namespace winrt::Windows::UI::Xaml;
    namespace WFM = winrt::Windows::Foundation::Metadata;
    using namespace winrt::Windows::Storage::Streams;
    using namespace ::FFmpegInteropX;

    enum ByteOrderMark
    {
        Unchecked,
        Unknown,
        UTF8
    };

    struct FFmpegMediaSource : FFmpegMediaSourceT<FFmpegMediaSource>
    {
        FFmpegMediaSource() = default;
        virtual ~FFmpegMediaSource();

        static Windows::Foundation::IAsyncOperation<FFmpegInteropX::FFmpegMediaSource> CreateFromStreamAsync(Windows::Storage::Streams::IRandomAccessStream stream, FFmpegInteropX::MediaSourceConfig config);
        static Windows::Foundation::IAsyncOperation<FFmpegInteropX::FFmpegMediaSource> CreateFromStreamAsync(Windows::Storage::Streams::IRandomAccessStream stream);
        static Windows::Foundation::IAsyncOperation<FFmpegInteropX::FFmpegMediaSource> CreateFromUriAsync(hstring uri, FFmpegInteropX::MediaSourceConfig config);
        static Windows::Foundation::IAsyncOperation<FFmpegInteropX::FFmpegMediaSource> CreateFromUriAsync(hstring uri);
        void SetSubtitleDelay(Windows::Foundation::TimeSpan const& delay);
        void SetFFmpegAudioFilters(hstring const& audioFilters);
        void SetFFmpegVideoFilters(hstring const& videoEffects);
        void DisableAudioEffects();
        void DisableVideoEffects();
        FFmpegInteropX::MediaThumbnailData ExtractThumbnail();
        Windows::Media::Core::MediaStreamSource GetMediaStreamSource();
        Windows::Media::Playback::MediaPlaybackItem CreateMediaPlaybackItem();
        Windows::Media::Playback::MediaPlaybackItem CreateMediaPlaybackItem(Windows::Foundation::TimeSpan const& startTime);
        Windows::Media::Playback::MediaPlaybackItem CreateMediaPlaybackItem(Windows::Foundation::TimeSpan const& startTime, Windows::Foundation::TimeSpan const& durationLimit);
        Windows::Foundation::IAsyncOperation<Windows::Foundation::Collections::IVectorView<FFmpegInteropX::SubtitleStreamInfo>> AddExternalSubtitleAsync(Windows::Storage::Streams::IRandomAccessStream stream, hstring streamName);
        Windows::Foundation::IAsyncOperation<Windows::Foundation::Collections::IVectorView<FFmpegInteropX::SubtitleStreamInfo>> AddExternalSubtitleAsync(Windows::Storage::Streams::IRandomAccessStream stream);
        FFmpegInteropX::MediaSourceConfig Configuration();
        winrt::Windows::Foundation::Collections::IMapView<hstring, winrt::Windows::Foundation::Collections::IVectorView<hstring>> MetadataTags();
        Windows::Foundation::TimeSpan Duration();
        FFmpegInteropX::VideoStreamInfo CurrentVideoStream();
        FFmpegInteropX::AudioStreamInfo CurrentAudioStream();
        Windows::Foundation::Collections::IVectorView<FFmpegInteropX::VideoStreamInfo> VideoStreams();
        Windows::Foundation::Collections::IVectorView<FFmpegInteropX::AudioStreamInfo> AudioStreams();
        Windows::Foundation::Collections::IVectorView<FFmpegInteropX::SubtitleStreamInfo> SubtitleStreams();
        Windows::Foundation::Collections::IVectorView<FFmpegInteropX::ChapterInfo> ChapterInfos();
        FFmpegInteropX::FormatInfo FormatInfo();
        bool HasThumbnail();
        Windows::Media::Playback::MediaPlaybackItem PlaybackItem();
        Windows::Foundation::TimeSpan SubtitleDelay();
        Windows::Foundation::TimeSpan BufferTime();
        void BufferTime(winrt::Windows::Foundation::TimeSpan const& value);
        Windows::Media::Playback::MediaPlaybackSession PlaybackSession();
        void PlaybackSession(Windows::Media::Playback::MediaPlaybackSession const& value);
        void Close();
        FFmpegMediaSource(winrt::com_ptr<MediaSourceConfig> const& interopConfig, CoreDispatcher const& dispatcher);

    private:

        HRESULT CreateMediaStreamSource(IRandomAccessStream const& stream);
        HRESULT CreateMediaStreamSource(hstring const& uri);
        HRESULT InitFFmpegContext();
        MediaSource CreateMediaSource();
        std::shared_ptr<MediaSampleProvider> CreateAudioStream(AVStream* avStream, int index);
        std::shared_ptr<MediaSampleProvider> CreateVideoStream(AVStream* avStream, int index);
        std::shared_ptr<SubtitleProvider> CreateSubtitleSampleProvider(AVStream* avStream, int index);
        std::shared_ptr<MediaSampleProvider> CreateAudioSampleProvider(AVStream* avStream, AVCodecContext* avCodecCtx, int index);
        std::shared_ptr<MediaSampleProvider> CreateVideoSampleProvider(AVStream* avStream, AVCodecContext* avCodecCtx, int index);
        HRESULT ParseOptions(PropertySet const& ffmpegOptions);
        void OnStarting(MediaStreamSource const& sender, MediaStreamSourceStartingEventArgs const& args);
        void OnSampleRequested(MediaStreamSource const& sender, MediaStreamSourceSampleRequestedEventArgs const& args);
        void CheckVideoDeviceChanged();
        void OnSwitchStreamsRequested(MediaStreamSource  const& sender, MediaStreamSourceSwitchStreamsRequestedEventArgs  const& args);
        void OnAudioTracksChanged(MediaPlaybackItem  const& sender, IVectorChangedEventArgs  const& args);
        void OnPresentationModeChanged(MediaPlaybackTimedMetadataTrackList  const& sender, TimedMetadataPresentationModeChangedEventArgs  const& args);
        void InitializePlaybackItem(MediaPlaybackItem  const& playbackitem);
        bool CheckUseHardwareAcceleration(AVCodecContext* avCodecCtx, HardwareAccelerationStatus status, HardwareDecoderStatus& hardwareDecoderStatus, int maxProfile, int maxLevel);

        void FlushStreams()
        {
            // Flush all active streams
            for each (auto stream in sampleProviders)
            {
                if (stream && stream->IsEnabled())
                {
                    stream->Flush();
                }
            }
        }

    public://internal:
        static winrt::com_ptr<FFmpegMediaSource> CreateFromStream(IRandomAccessStream const& stream, winrt::com_ptr<MediaSourceConfig> const& config, CoreDispatcher  const& dispatcher);
        static winrt::com_ptr<FFmpegMediaSource> CreateFromUri(hstring  const& uri, winrt::com_ptr<MediaSourceConfig>  const& config, CoreDispatcher  const& dispatcher);
        static winrt::com_ptr<FFmpegMediaSource> CreateFromUri(hstring  const& uri, winrt::com_ptr<MediaSourceConfig>  const& config);
        HRESULT Seek(TimeSpan position, TimeSpan& actualPosition, bool allowFastSeek);

        std::shared_ptr<MediaSampleProvider> VideoSampleProvider()
        {
            return currentVideoStream;
        }

        std::shared_ptr<FFmpegReader> m_pReader;
        AVDictionary* avDict = NULL;
        AVIOContext* avIOCtx = NULL;
        AVFormatContext* avFormatCtx = NULL;
        winrt::com_ptr<IStream> fileStreamData;
        ByteOrderMark streamByteOrderMark;
        winrt::com_ptr<MediaSourceConfig> config = { nullptr };

    private:

        MediaStreamSource mss = { nullptr };
        winrt::event_token startingRequestedToken{};
        winrt::event_token sampleRequestedToken{};
        winrt::event_token switchStreamRequestedToken{};
        MediaPlaybackItem playbackItem = { nullptr };
        IVector<winrt::FFmpegInteropX::AudioStreamInfo> audioStrInfos = { nullptr };
        IVector<winrt::FFmpegInteropX::SubtitleStreamInfo> subtitleStrInfos = { nullptr };
        IVector<winrt::FFmpegInteropX::VideoStreamInfo> videoStrInfos = { nullptr };

        std::vector<std::shared_ptr<MediaSampleProvider>> sampleProviders;
        std::vector<std::shared_ptr<MediaSampleProvider>> audioStreams;
        std::vector< std::shared_ptr<SubtitleProvider>> subtitleStreams;
        std::vector<std::shared_ptr<MediaSampleProvider>> videoStreams;

        std::shared_ptr<MediaSampleProvider> currentVideoStream;
        std::shared_ptr<MediaSampleProvider> currentAudioStream;
        hstring currentAudioEffects{};
        int thumbnailStreamIndex = 0;

        winrt::event_token audioTracksChangedToken{};
        winrt::event_token subtitlePresentationModeChangedToken{};


        IVectorView<winrt::FFmpegInteropX::VideoStreamInfo> videoStreamInfos = { nullptr };
        IVectorView<winrt::FFmpegInteropX::AudioStreamInfo> audioStreamInfos = { nullptr };
        IVectorView<winrt::FFmpegInteropX::SubtitleStreamInfo> subtitleStreamInfos = { nullptr };
        IVectorView<winrt::FFmpegInteropX::ChapterInfo> chapterInfos = { nullptr };
        winrt::FFmpegInteropX::FormatInfo formatInfo = { nullptr };

        std::shared_ptr<AttachedFileHelper> attachedFileHelper = NULL;

        std::shared_ptr<MediaMetadata> metadata = NULL;

        std::recursive_mutex mutexGuard;
        CoreDispatcher dispatcher = { nullptr };
        MediaPlaybackSession session = { nullptr };
        winrt::event_token sessionPositionEvent{};

        hstring videoCodecName{};
        hstring audioCodecName{};
        TimeSpan mediaDuration{};
        TimeSpan subtitleDelay{};
        unsigned char* fileStreamBuffer = NULL;
        bool isFirstSeek;
        AVBufferRef* avHardwareContext = NULL;
        AVBufferRef* avHardwareContextDefault = NULL;
        ID3D11Device* device = NULL;
        ID3D11DeviceContext* deviceContext = NULL;
        HANDLE deviceHandle = NULL;
        IMFDXGIDeviceManager* deviceManager = NULL;

        bool isFirstSeekAfterStreamSwitch = false;
        bool isLastSeekForward = false;
        TimeSpan lastSeekStart{};
        TimeSpan lastSeekActual{};

        TimeSpan currentPosition{};
        TimeSpan lastPosition{};

        static CoreDispatcher GetCurrentDispatcher();
        void OnPositionChanged(Windows::Media::Playback::MediaPlaybackSession const& sender, winrt::Windows::Foundation::IInspectable const& args);
    };
}
namespace winrt::FFmpegInteropX::factory_implementation
{
    struct FFmpegMediaSource : FFmpegMediaSourceT<FFmpegMediaSource, implementation::FFmpegMediaSource>
    {
    };
}
