#pragma once
#include "FFmpegMediaSource.g.h"
#include <pch.h>
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

namespace winrt::FFmpegInteropXWinUI::implementation
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
	using namespace FFmpegInteropX;

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
		FFmpegMediaSource(MediaSourceConfig const& interopConfig, CoreDispatcher const& dispatcher);

	private:

		HRESULT CreateMediaStreamSource(IRandomAccessStream const& stream);
		HRESULT CreateMediaStreamSource(hstring const& uri);
		HRESULT InitFFmpegContext();
		MediaSource CreateMediaSource();
		MediaSampleProvider CreateAudioStream(AVStream* avStream, int index);
		MediaSampleProvider CreateVideoStream(AVStream* avStream, int index);
		SubtitleProvider CreateSubtitleSampleProvider(AVStream* avStream, int index);
		MediaSampleProvider CreateAudioSampleProvider(AVStream* avStream, AVCodecContext* avCodecCtx, int index);
		MediaSampleProvider CreateVideoSampleProvider(AVStream* avStream, AVCodecContext* avCodecCtx, int index);
		HRESULT ParseOptions(PropertySet ffmpegOptions);
		void OnStarting(MediaStreamSource sender, MediaStreamSourceStartingEventArgs args);
		void OnSampleRequested(MediaStreamSource sender, MediaStreamSourceSampleRequestedEventArgs args);
		void CheckVideoDeviceChanged();
		void OnSwitchStreamsRequested(MediaStreamSource sender, MediaStreamSourceSwitchStreamsRequestedEventArgs args);
		void OnAudioTracksChanged(MediaPlaybackItem sender, IVectorChangedEventArgs args);
		void OnPresentationModeChanged(MediaPlaybackTimedMetadataTrackList sender, TimedMetadataPresentationModeChangedEventArgs args);
		void InitializePlaybackItem(MediaPlaybackItem playbackitem);
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
		static winrt::com_ptr<FFmpegMediaSource> CreateFromStream(IRandomAccessStream const& stream, MediaSourceConfig  const& config, CoreDispatcher  const& dispatcher);
		static winrt::com_ptr<FFmpegMediaSource> CreateFromUri(hstring  const& uri, MediaSourceConfig  const& config, CoreDispatcher  const& dispatcher);
		static winrt::com_ptr<FFmpegMediaSource> CreateFromUri(hstring  const& uri, MediaSourceConfig  const& config);
		HRESULT Seek(TimeSpan position, TimeSpan& actualPosition, bool allowFastSeek);

		std::shared_ptr<MediaSampleProvider> VideoSampleProvider()
		{
			return currentVideoStream;
		}

		std::shared_ptr<FFmpegReader> m_pReader;
		AVDictionary* avDict;
		AVIOContext* avIOCtx;
		AVFormatContext* avFormatCtx;
		IStream* fileStreamData;
		ByteOrderMark streamByteOrderMark;
		winrt::FFmpegInteropXWinUI::MediaSourceConfig config = { nullptr };

	private:

		MediaStreamSource mss = { nullptr };
		winrt::event_token startingRequestedToken;
		winrt::event_token sampleRequestedToken;
		winrt::event_token switchStreamRequestedToken;
		MediaPlaybackItem playbackItem = { nullptr };
		IVector<winrt::FFmpegInteropXWinUI::AudioStreamInfo> audioStrInfos = { nullptr };
		IVector<winrt::FFmpegInteropXWinUI::SubtitleStreamInfo> subtitleStrInfos = { nullptr };
		IVector<winrt::FFmpegInteropXWinUI::VideoStreamInfo> videoStrInfos = { nullptr };

		std::vector<std::shared_ptr<MediaSampleProvider>> sampleProviders = { nullptr };
		std::vector<std::shared_ptr<MediaSampleProvider>> audioStreams = { nullptr };
		std::vector< std::shared_ptr<SubtitleProvider>> subtitleStreams = { nullptr };
		std::vector<std::shared_ptr<MediaSampleProvider>> videoStreams = { nullptr };

		std::shared_ptr<MediaSampleProvider> currentVideoStream;
		std::shared_ptr<MediaSampleProvider> currentAudioStream;
		hstring currentAudioEffects;
		int thumbnailStreamIndex;

		winrt::event_token audioTracksChangedToken;
		winrt::event_token subtitlePresentationModeChangedToken;


		IVectorView<winrt::FFmpegInteropXWinUI::VideoStreamInfo> videoStreamInfos = { nullptr };
		IVectorView<winrt::FFmpegInteropXWinUI::AudioStreamInfo> audioStreamInfos = { nullptr };
		IVectorView<winrt::FFmpegInteropXWinUI::SubtitleStreamInfo> subtitleStreamInfos = { nullptr };
		IVectorView<winrt::FFmpegInteropXWinUI::ChapterInfo> chapterInfos = { nullptr };
		winrt::FFmpegInteropXWinUI::FormatInfo formatInfo = { nullptr };

		std::shared_ptr<AttachedFileHelper> attachedFileHelper;

		std::shared_ptr<MediaMetadata> metadata;

		std::recursive_mutex mutexGuard;
		CoreDispatcher dispatcher = { nullptr };
		MediaPlaybackSession session = { nullptr };
		winrt::event_token sessionPositionEvent;

		hstring videoCodecName;
		hstring audioCodecName;
		TimeSpan mediaDuration;
		TimeSpan subtitleDelay;
		unsigned char* fileStreamBuffer;
		bool isFirstSeek;
		AVBufferRef* avHardwareContext;
		AVBufferRef* avHardwareContextDefault;
		ID3D11Device* device;
		ID3D11DeviceContext* deviceContext;
		HANDLE deviceHandle;
		IMFDXGIDeviceManager* deviceManager;

		bool isFirstSeekAfterStreamSwitch;
		bool isLastSeekForward;
		TimeSpan lastSeekStart;
		TimeSpan lastSeekActual;

		TimeSpan actualPosition;
		TimeSpan lastPosition;

		static CoreDispatcher GetCurrentDispatcher();
		void OnPositionChanged(Windows::Media::Playback::MediaPlaybackSession const& sender, winrt::Windows::Foundation::IInspectable const& args);
	};
}
namespace winrt::FFmpegInteropXWinUI::factory_implementation
{
	struct FFmpegMediaSource : FFmpegMediaSourceT<FFmpegMediaSource, implementation::FFmpegMediaSource>
	{
	};
}
