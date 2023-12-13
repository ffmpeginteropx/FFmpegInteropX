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
#include "text_encoding_detect.h"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropX::implementation
{
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Windows::Foundation::Collections;
    using namespace winrt::Windows::Media::Core;
    using namespace winrt::Windows::Media::Playback;
    using namespace winrt::Windows::Storage::Streams;

#ifdef WinUI
    using namespace winrt::Microsoft::UI::Dispatching;
#else
    using namespace winrt::Windows::System;
    using namespace TextEncoding;
#endif

    struct FFmpegMediaSource : FFmpegMediaSourceT<FFmpegMediaSource>
    {
        virtual ~FFmpegMediaSource();

#ifdef WinUI
        ///<summary>Creates a FFmpegMediaSource from a stream.</summary>
        static IAsyncOperation<FFmpegInteropX::FFmpegMediaSource> CreateFromStreamAsync(IRandomAccessStream stream, FFmpegInteropX::MediaSourceConfig config, Microsoft::UI::WindowId windowId)
        {
            return CreateFromStreamAsync(stream, config, windowId.Value);
        }

        ///<summary>Creates a FFmpegMediaSource from a Uri.</summary>
        static IAsyncOperation<FFmpegInteropX::FFmpegMediaSource> CreateFromUriAsync(hstring uri, FFmpegInteropX::MediaSourceConfig config, Microsoft::UI::WindowId windowId)
        {
            return CreateFromUriAsync(uri, config, windowId.Value);
        }

        ///<summary>Creates a FFmpegMediaSource from a file.</summary>
        static IAsyncOperation<FFmpegInteropX::FFmpegMediaSource> CreateFromFileAsync(hstring fileName, FFmpegInteropX::MediaSourceConfig config, Microsoft::UI::WindowId windowId)
        {
            return CreateFromUriAsync(fileName, config, windowId.Value);
        }
#endif


        ///<summary>Creates a FFmpegMediaSource from a stream.</summary>
        static IAsyncOperation<FFmpegInteropX::FFmpegMediaSource> CreateFromStreamAsync(IRandomAccessStream stream, FFmpegInteropX::MediaSourceConfig config)
        {
            return CreateFromStreamAsync(stream, config, 0);
        }

        ///<summary>Creates a FFmpegMediaSource from a stream.</summary>
        static IAsyncOperation<FFmpegInteropX::FFmpegMediaSource> CreateFromStreamAsync(IRandomAccessStream stream)
        {
            return CreateFromStreamAsync(stream, FFmpegInteropX::MediaSourceConfig(), 0);
        }

        ///<summary>Creates a FFmpegMediaSource from a Uri.</summary>
        static IAsyncOperation<FFmpegInteropX::FFmpegMediaSource> CreateFromUriAsync(hstring uri, FFmpegInteropX::MediaSourceConfig config)
        {
            return CreateFromUriAsync(uri, config, 0);
        }

        ///<summary>Creates a FFmpegMediaSource from a Uri.</summary>
        static IAsyncOperation<FFmpegInteropX::FFmpegMediaSource> FFmpegMediaSource::CreateFromUriAsync(hstring uri)
        {
            return CreateFromUriAsync(uri, FFmpegInteropX::MediaSourceConfig(), 0);
        }

#ifndef UWP
        ///<summary>Creates a FFmpegMediaSource from a file.</summary>
        static IAsyncOperation<FFmpegInteropX::FFmpegMediaSource> CreateFromFileAsync(hstring fileName, FFmpegInteropX::MediaSourceConfig config)
        {
            return CreateFromUriAsync(fileName, config, 0);
        }

        ///<summary>Creates a FFmpegMediaSource from a file.</summary>
        static IAsyncOperation<FFmpegInteropX::FFmpegMediaSource> CreateFromFileAsync(hstring fileName)
        {
            return CreateFromUriAsync(fileName, FFmpegInteropX::MediaSourceConfig(), 0);
        }
#endif

        ///<summary>Sets the subtitle delay for all subtitle streams. Use negative values to speed them up, positive values to delay them.</summary>
        void SetSubtitleDelay(TimeSpan const& delay);

        ///<summary>Sets FFmpeg audio filters. This replaces any filters which were already set.</summary>
        void SetFFmpegAudioFilters(hstring const& audioFilters);

        ///<summary>Sets FFmpeg audio filters for audio stream specified by audioStreamIndex. This replaces any filters which were already set.</summary>
        void SetFFmpegAudioFilters(hstring const& audioFilters, winrt::FFmpegInteropX::AudioStreamInfo const& audioStream);

        ///<summary>Sets FFmpeg video filters. This replaces any filters which were already set.</summary>
        ///<remarks>Using FFmpeg video filters will degrade playback performance, since they run on the CPU and not on the GPU.</remarks>
        void SetFFmpegVideoFilters(hstring const& videoFilters);

        ///<summary>Sets FFmpeg video filters for the video stream specified by videoStreamIndex. This replaces any filters which were already set.</summary>
        ///<remarks>Using FFmpeg video filters will degrade playback performance, since they run on the CPU and not on the GPU.</remarks>
        void SetFFmpegVideoFilters(hstring const& videoFilters, winrt::FFmpegInteropX::VideoStreamInfo const& videoStream);

        ///<summary>Disables audio filters.</summary>
        void ClearFFmpegAudioFilters();

        ///<summary>Disables audio filters for the specified audio stream.</summary>
        void ClearFFmpegAudioFilters(winrt::FFmpegInteropX::AudioStreamInfo const& audioStream);

        ///<summary>Clears video filters.</summary>
        void ClearFFmpegVideoFilters();

        ///<summary>Disables audio filters for the specified video stream.</summary>
        void ClearFFmpegVideoFilters(winrt::FFmpegInteropX::VideoStreamInfo const& videoStream);

        ///<summary>Gets audio filters for the specified audio stream.</summary>
        hstring GetFFmpegAudioFilters(winrt::FFmpegInteropX::AudioStreamInfo const& audioStream);

        ///<summary>Gets video filters for the specified video stream.</summary>
        hstring GetFFmpegVideoFilters(winrt::FFmpegInteropX::VideoStreamInfo const& videoStream);

        ///<summary>Extracts an embedded thumbnail, if one is available (see HasThumbnail).</summary>
        FFmpegInteropX::MediaThumbnailData ExtractThumbnail();

        ///<summary>Gets the MediaStreamSource. Using the MediaStreamSource will prevent subtitles from working. Please use CreateMediaPlaybackItem instead.</summary>
        MediaStreamSource GetMediaStreamSource();

        ///<summary>Creates a MediaPlaybackItem for playback.</summary>
        MediaPlaybackItem CreateMediaPlaybackItem();

        ///<summary>Creates a MediaPlaybackItem for playback which starts at the specified stream offset.</summary>
        MediaPlaybackItem CreateMediaPlaybackItem(TimeSpan const& startTime);

        ///<summary>Creates a MediaPlaybackItem for playback which starts at the specified stream offset and ends after the specified duration.</summary>
        MediaPlaybackItem CreateMediaPlaybackItem(TimeSpan const& startTime, TimeSpan const& durationLimit);

        ///<summary>Creates a MediaPlaybackItem, assigns it to MediaPlayer.Source and waits for MediaOpened or MediaFailed (throws in that case).</summary>
        ///<remarks>This will also automatically cleanup resources, if MediaPlayer switches to a different file, or it's Source property is assigned null.</remarks>
        IAsyncAction OpenWithMediaPlayerAsync(MediaPlayer mediaPlayer);

        ///<summary>Adds an external subtitle from a stream.</summary>
        ///<param name="stream">The subtitle stream.</param>
        ///<param name="streamName">The name to use for the subtitle.</param>
        IAsyncOperation<IVectorView<FFmpegInteropX::SubtitleStreamInfo>> AddExternalSubtitleAsync(IRandomAccessStream stream, hstring streamName);

        ///<summary>Adds an external subtitle from a stream.</summary>
        ///<param name="stream">The subtitle stream.</param>
        IAsyncOperation<IVectorView<FFmpegInteropX::SubtitleStreamInfo>> AddExternalSubtitleAsync(IRandomAccessStream stream);

        ///<summary>Starts filling the read-ahead buffer, if enabled in the configuration.</summary>
        ///<remarks>Let the stream buffer fill before starting playback.</remarks>
        void StartBuffering();

        ///<summary>Gets the configuration that has been passed when creating the MSS instance.</summary>
        FFmpegInteropX::MediaSourceConfig Configuration();

        ///<summary>Gets the metadata tags available in the file.</summary>
        IMapView<hstring, IVectorView<hstring>> MetadataTags();

        ///<summary>Gets the duration of the stream. Returns zero, if this is streaming media.</summary>
        TimeSpan Duration();

        ///<summary>Gets the current video stream information.</summary>
        FFmpegInteropX::VideoStreamInfo CurrentVideoStream();

        ///<summary>Gets the current audio stream information.</summary>
        FFmpegInteropX::AudioStreamInfo CurrentAudioStream();

        ///<summary>Gets video stream information</summary>
        IVectorView<FFmpegInteropX::VideoStreamInfo> VideoStreams();

        ///<summary>Gets audio stream information.</summary>
        IVectorView<FFmpegInteropX::AudioStreamInfo> AudioStreams();

        ///<summary>Gets subtitle stream information.</summary>
        IVectorView<FFmpegInteropX::SubtitleStreamInfo> SubtitleStreams();

        ///<summary>Gets chapter information.</summary>
        IVectorView<FFmpegInteropX::ChapterInfo> ChapterInfos();

        ///<summary>Gets format information.</summary>
        FFmpegInteropX::FormatInfo FormatInfo();

        ///<summary>Gets a boolean indication if a thumbnail is embedded in the file.</summary>
        bool HasThumbnail();

        ///<summary>Gets the MediaPlaybackItem that was created before by using CreateMediaPlaybackItem.</summary>
        MediaPlaybackItem PlaybackItem();

        ///<summary>Gets or sets the MediaPlaybackSession associated with this FFmpeg source. Used when FastSeek is enabled.</summary>
        ///<remarks>After playback has started, please assign MediaPlayer.PlaybackSession to this .</remarks>
        MediaPlaybackSession PlaybackSession();
        void PlaybackSession(MediaPlaybackSession const& value);

        ///<summary>Sets a presentation timestamp delay for the given stream. Audio, video and subtitle synchronisation can be achieved this way. A positive value will cause samples (or subtitles) to be rendered at a later time. A negative value will make rendering come sooner</summary>
        void SetStreamDelay(winrt::FFmpegInteropX::IStreamInfo const& stream, winrt::Windows::Foundation::TimeSpan const& delay);

        ///<summary>Gets the presentation timestamp delay for the given stream. </summary>
        winrt::Windows::Foundation::TimeSpan GetStreamDelay(winrt::FFmpegInteropX::IStreamInfo const& stream);

        void Close();

        FFmpegMediaSource(winrt::com_ptr<MediaSourceConfig> const& interopConfig, DispatcherQueue const& dispatcher, uint64_t windowId, bool useHdr);

    private:

        HRESULT CreateMediaStreamSource(IRandomAccessStream const& stream);
        HRESULT CreateMediaStreamSource(hstring const& uri);
        HRESULT InitFFmpegContext();
        MediaStreamSource CreateMediaStreamSource();
        MediaSource CreateMediaSource();
        std::shared_ptr<MediaSampleProvider> CreateAudioStream(AVStream* avStream, int index);
        std::shared_ptr<MediaSampleProvider> CreateVideoStream(AVStream* avStream, int index);
        std::shared_ptr<SubtitleProvider> CreateSubtitleSampleProvider(AVStream* avStream, int index);
        std::shared_ptr<MediaSampleProvider> CreateAudioSampleProvider(AVStream* avStream, AVCodecContext* avCodecCtx, int index);
        std::shared_ptr<MediaSampleProvider> CreateVideoSampleProvider(AVStream* avStream, AVCodecContext* avCodecCtx, int index);
        HRESULT ParseOptions(PropertySet const& ffmpegOptions);
        void MediaStreamSourceClosed(MediaStreamSource const& sender, MediaStreamSourceClosedEventArgs const& args);
        void OnStarting(MediaStreamSource const& sender, MediaStreamSourceStartingEventArgs const& args);
        void OnSampleRequested(MediaStreamSource const& sender, MediaStreamSourceSampleRequestedEventArgs const& args);
        void CheckVideoDeviceChanged(MediaStreamSource const& mss);
        void OnSwitchStreamsRequested(MediaStreamSource  const& sender, MediaStreamSourceSwitchStreamsRequestedEventArgs  const& args);
        void OnAudioTracksChanged(MediaPlaybackItem  const& sender, IVectorChangedEventArgs  const& args);
        void OnPresentationModeChanged(MediaPlaybackTimedMetadataTrackList  const& sender, TimedMetadataPresentationModeChangedEventArgs  const& args);
        void InitializePlaybackItem(MediaPlaybackItem  const& playbackitem);
        bool CheckUseHardwareAcceleration(AVCodecContext* avCodecCtx, HardwareAccelerationStatus const& status, HardwareDecoderStatus& hardwareDecoderStatus, int maxProfile, int maxLevel);
        static void CheckUseHdr(winrt::com_ptr<MediaSourceConfig> const& config, bool checkDisplayInformation, bool& useHdr, uint64_t& windowId);
        void CheckExtendDuration(MediaStreamSample sample);
        MediaThumbnailData ExtractThumbnail(AVStream* avStream);

        void Close(bool onMediaSourceClosed);
        static DispatcherQueue GetCurrentDispatcherQueue();

    public://internal:
        static IAsyncOperation<FFmpegInteropX::FFmpegMediaSource> CreateFromStreamAsync(IRandomAccessStream stream, FFmpegInteropX::MediaSourceConfig config, uint64_t windowId);
        static IAsyncOperation<FFmpegInteropX::FFmpegMediaSource> CreateFromUriAsync(hstring uri, FFmpegInteropX::MediaSourceConfig config, uint64_t windowId);
        static winrt::com_ptr<FFmpegMediaSource> CreateFromStream(IRandomAccessStream const& stream, winrt::com_ptr<MediaSourceConfig> const& config, DispatcherQueue  const& dispatcher, uint64_t windowId, bool useHdr);
        static winrt::com_ptr<FFmpegMediaSource> CreateFromUri(hstring  const& uri, winrt::com_ptr<MediaSourceConfig>  const& config, DispatcherQueue  const& dispatcher, uint64_t windowId, bool useHdr);
        HRESULT Seek(TimeSpan const& position, TimeSpan& actualPosition, bool allowFastSeek);

        std::shared_ptr<MediaSampleProvider> VideoSampleProvider()
        {
            return currentVideoStream;
        }

        std::shared_ptr<FFmpegReader> m_pReader;
        AVDictionary* avDict = nullptr;
        AVIOContext* avIOCtx = nullptr;
        AVFormatContext* avFormatCtx = nullptr;
        winrt::com_ptr<IStream> fileStreamData = { nullptr };
        TextEncodingDetect::Encoding streamEncoding = TextEncodingDetect::None;
        bool streamEncodingChecked = false;
        winrt::com_ptr<MediaSourceConfig> config = { nullptr };
        bool isShuttingDown = false;

    private:

        winrt::weak_ref<MediaStreamSource> mssWeak = { nullptr };
        winrt::event_token startingRequestedToken{};
        winrt::event_token sampleRequestedToken{};
        winrt::event_token switchStreamRequestedToken{};
        winrt::event_token closeToken{};

        winrt::weak_ref<MediaPlaybackItem> playbackItemWeak = { nullptr };
        IVector<FFmpegInteropX::AudioStreamInfo> audioStrInfos = { nullptr };
        IVector<FFmpegInteropX::SubtitleStreamInfo> subtitleStrInfos = { nullptr };
        IVector<FFmpegInteropX::VideoStreamInfo> videoStrInfos = { nullptr };

        std::vector<std::shared_ptr<MediaSampleProvider>> sampleProviders;
        std::vector<std::shared_ptr<MediaSampleProvider>> audioStreams;
        std::vector<std::shared_ptr<SubtitleProvider>> subtitleStreams;
        std::vector<std::shared_ptr<MediaSampleProvider>> videoStreams;

        std::shared_ptr<MediaSampleProvider> currentVideoStream = { nullptr };
        std::shared_ptr<MediaSampleProvider> currentAudioStream = { nullptr };
        FFmpegInteropX::AudioStreamInfo currentAudioStreamInfo = { nullptr };
        FFmpegInteropX::VideoStreamInfo currentVideoStreamInfo = { nullptr };
        int thumbnailStreamIndex = 0;

        winrt::event_token audioTracksChangedToken{};
        winrt::event_token subtitlePresentationModeChangedToken{};

        IVectorView<FFmpegInteropX::VideoStreamInfo> videoStreamInfos = { nullptr };
        IVectorView<FFmpegInteropX::AudioStreamInfo> audioStreamInfos = { nullptr };
        IVectorView<FFmpegInteropX::SubtitleStreamInfo> subtitleStreamInfos = { nullptr };
        IVectorView<FFmpegInteropX::ChapterInfo> chapterInfos = { nullptr };
        FFmpegInteropX::FormatInfo formatInfo = { nullptr };

        std::shared_ptr<AttachedFileHelper> attachedFileHelper = { nullptr };

        std::shared_ptr<MediaMetadata> metadata = { nullptr };
        MediaThumbnailData thumbnailData = nullptr;

        std::recursive_mutex mutex;
        DispatcherQueue dispatcher = { nullptr };
        winrt::weak_ref<MediaPlaybackSession> sessionWeak = { nullptr };
        winrt::event_token sessionPositionEvent{};

        TimeSpan mediaDuration{};

        AVBufferRef* avHardwareContext = nullptr;
        AVBufferRef* avHardwareContextDefault = nullptr;
        com_ptr<ID3D11Device> device;
        com_ptr<ID3D11DeviceContext> deviceContext;
        HANDLE deviceHandle = nullptr;
        com_ptr<IMFDXGIDeviceManager> deviceManager;

        uint64_t windowId;
        bool useHdr;

        bool isFirstSeek;
        bool isFirstSeekAfterStreamSwitch = false;

        int lastDurationExtension = 0;

        bool isClosed = false;

        TimeSpan currentPosition{ 0 };
        TimeSpan lastPosition{ 0 };
        TimeSpan lastSeek{ 0 };

        void OnPositionChanged(MediaPlaybackSession const& sender, IInspectable const& args);
    };
}
namespace winrt::FFmpegInteropX::factory_implementation
{
    struct FFmpegMediaSource : FFmpegMediaSourceT<FFmpegMediaSource, implementation::FFmpegMediaSource>
    {
    };
}
