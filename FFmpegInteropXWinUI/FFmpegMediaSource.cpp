#include "pch.h"
#include "FFmpegMediaSource.h"
#include "LanguageTagConverter.h"
#include "FFmpegMediaSource.g.cpp"
#include "winrt/Windows.ApplicationModel.Core.h"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropXWinUI::implementation
{
	// Static functions passed to FFmpeg
	static int FileStreamRead(void* ptr, uint8_t* buf, int bufSize)
	{
		FFmpegMediaSource* mss = reinterpret_cast<FFmpegMediaSource*>(ptr);
		ULONG bytesRead = 0;
		HRESULT hr = mss->fileStreamData->Read(buf, bufSize, &bytesRead);

		if (FAILED(hr))
		{
			return -1;
		}

		// Check beginning of file for BOM on first read
		if (mss->streamByteOrderMark == ByteOrderMark::Unchecked)
		{
			if (bytesRead >= 4)
			{
				auto bom = ((uint32_t*)buf)[0];
				if ((bom & 0x00FFFFFF) == 0x00BFBBEF)
				{
					mss->streamByteOrderMark = ByteOrderMark::UTF8;
				}
				else
				{
					mss->streamByteOrderMark = ByteOrderMark::Unknown;
				}
			}
			else
			{
				mss->streamByteOrderMark = ByteOrderMark::Unknown;
			}
		}

		// If we succeed but don't have any bytes, assume end of file
		if (bytesRead == 0)
		{
			return AVERROR_EOF;  // Let FFmpeg know that we have reached eof
		}

		return bytesRead;
	}

	// Static function to seek in file stream. Credit to Philipp Sch http://www.codeproject.com/Tips/489450/Creating-Custom-FFmpeg-IO-Context
	static int64_t FileStreamSeek(void* ptr, int64_t pos, int whence)
	{
		FFmpegMediaSource* mss = reinterpret_cast<FFmpegMediaSource*>(ptr);
		if (whence == AVSEEK_SIZE)
		{
			// get stream size
			STATSTG status;
			if (FAILED(mss->fileStreamData->Stat(&status, STATFLAG_NONAME)))
			{
				return -1;
			}
			return status.cbSize.QuadPart;
		}
		else
		{
			LARGE_INTEGER in;
			in.QuadPart = pos;
			ULARGE_INTEGER out = { 0 };

			if (FAILED(mss->fileStreamData->Seek(in, whence, &out)))
			{
				return -1;
			}

			return out.QuadPart; // Return the new position:
		}
	}

	// Flag for ffmpeg global setup
	static bool isRegistered = false;
	std::mutex isRegisteredMutex;

	FFmpegMediaSource::FFmpegMediaSource(MediaSourceConfig const& interopConfig,
		CoreDispatcher const& dispatcher)
		: config(interopConfig)
		, thumbnailStreamIndex(AVERROR_STREAM_NOT_FOUND)
		, isFirstSeek(true)
		, dispatcher(dispatcher)
	{
		if (!isRegistered)
		{
			isRegisteredMutex.lock();
			if (!isRegistered)
			{
				LanguageTagConverter::Initialize();
				isRegistered = true;
			}

			isRegisteredMutex.unlock();
		}
		subtitleDelay = config.DefaultSubtitleDelay();
		audioStrInfos = winrt::single_threaded_vector<winrt::FFmpegInteropXWinUI::AudioStreamInfo>();
		subtitleStrInfos = winrt::single_threaded_vector<winrt::FFmpegInteropXWinUI::SubtitleStreamInfo>();
		videoStrInfos = winrt::single_threaded_vector<winrt::FFmpegInteropXWinUI::VideoStreamInfo>();
		auto implConfig = config.as<winrt::FFmpegInteropXWinUI::implementation::MediaSourceConfig>();
		if (!implConfig->IsExternalSubtitleParser && !implConfig->IsFrameGrabber)
		{
			metadata = std::shared_ptr<MediaMetadata>(new MediaMetadata());
		}
	}

	FFmpegMediaSource::~FFmpegMediaSource()
	{
		Close();
	}

	winrt::com_ptr<FFmpegMediaSource> FFmpegMediaSource::CreateFromStream(IRandomAccessStream const& stream, MediaSourceConfig const& config, CoreDispatcher const& dispatcher)
	{
		auto interopMSS = winrt::make_self<FFmpegMediaSource>(config, dispatcher);
		auto hr = interopMSS->CreateMediaStreamSource(stream);
		if (!SUCCEEDED(hr))
		{
			throw_hresult(hr);
		}
		return interopMSS;
	}

	winrt::com_ptr<FFmpegMediaSource> FFmpegMediaSource::CreateFromUri(hstring const& uri, MediaSourceConfig const& config, CoreDispatcher const& dispatcher)
	{
		auto interopMSS = winrt::make_self<FFmpegMediaSource>(config, dispatcher);
		auto hr = interopMSS->CreateMediaStreamSource(uri);
		if (!SUCCEEDED(hr))
		{
			throw_hresult(hr);
		}
		return interopMSS;
	}

	winrt::com_ptr<FFmpegMediaSource> FFmpegMediaSource::CreateFromUri(hstring const& uri, MediaSourceConfig const& config)
	{
		auto dispatcher = GetCurrentDispatcher();
		auto interopMSS = winrt::make_self<FFmpegMediaSource>(config, dispatcher);
		auto hr = interopMSS->CreateMediaStreamSource(uri);
		if (!SUCCEEDED(hr))
		{
			throw_hresult(hr);
		}
		return interopMSS;
	}

	HRESULT FFmpegMediaSource::CreateMediaStreamSource(IRandomAccessStream const& stream)
	{
		return S_OK;
	}

	HRESULT FFmpegMediaSource::CreateMediaStreamSource(hstring const& uri)
	{
		return S_OK;
	}

	CoreDispatcher FFmpegMediaSource::GetCurrentDispatcher()
	{
		try {
			//try get the current view
			auto wnd = CoreWindow::GetForCurrentThread();
			if (wnd == nullptr)
			{
				wnd = winrt::Windows::ApplicationModel::Core::CoreApplication::MainView().CoreWindow();
			}
			if (wnd != nullptr)
				return wnd.Dispatcher();

			return nullptr;
		}
		catch (...)
		{
			return nullptr;
		}
	}

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

	void FFmpegMediaSource::PlaybackSession(Windows::Media::Playback::MediaPlaybackSession const& value)
	{
		throw hresult_not_implemented();
	}

	void FFmpegMediaSource::Close()
	{
		mutexGuard.lock();
		if (mss)
		{
			mss.Starting(startingRequestedToken);
			mss.SampleRequested(sampleRequestedToken);
			mss.SwitchStreamsRequested(switchStreamRequestedToken);
			mss = nullptr;
		}

		if (playbackItem)
		{
			playbackItem.AudioTracksChanged(audioTracksChangedToken);
			playbackItem.TimedMetadataTracks().PresentationModeChanged(subtitlePresentationModeChangedToken);
			playbackItem = nullptr;
		}

		// Clear our data
		currentAudioStream.reset();
		currentVideoStream.reset();

		if (m_pReader != nullptr)
		{
			m_pReader.reset();;
		}

		for (auto x : subtitleStreams)
			x.reset();
		for (auto x : sampleProviders)
			x.reset();
		for (auto x : audioStreams)
			x.reset();
		for (auto x : videoStreams)
			x.reset();

		subtitleStreams.clear();
		sampleProviders.clear();
		audioStreams.clear();
		videoStreams.clear();

		avformat_close_input(&avFormatCtx);
		av_free(avIOCtx);
		av_dict_free(&avDict);

		if (fileStreamData != nullptr)
		{
			fileStreamData->Release();
		}
		if (avHardwareContext)
		{
			av_buffer_unref(&avHardwareContext);
		}
		if (avHardwareContextDefault)
		{
			av_buffer_unref(&avHardwareContextDefault);
		}

		if (deviceHandle && deviceManager)
			deviceManager->CloseDeviceHandle(deviceHandle);

		SAFE_RELEASE(device);
		SAFE_RELEASE(deviceContext);
		SAFE_RELEASE(deviceManager);

		PlaybackSession(nullptr);

		mutexGuard.unlock();
	}

	HRESULT FFmpegMediaSource::Seek(TimeSpan position, TimeSpan& actualPosition, bool allowFastSeek)
	{
		auto hr = S_OK;

		// Select the first valid stream either from video or audio
		auto stream = currentVideoStream ? currentVideoStream : currentAudioStream;

		if (stream)
		{
			int64_t seekTarget = stream->ConvertPosition(position);
			auto diffActual = position - actualPosition;
			auto diffLast = position - lastPosition;
			bool isSeekBeforeStreamSwitch = PlaybackSession() && config.FastSeekSmartStreamSwitching() && diffActual.count() > 0 && diffActual.count() < 5000000 && diffLast.count() > 0 && diffLast.count() < 10000000;

			if (currentVideoStream && config.FastSeek() && allowFastSeek && PlaybackSession() && !isSeekBeforeStreamSwitch && !isFirstSeekAfterStreamSwitch)
			{
				// fast seek
				auto playbackPosition = PlaybackSession() ? lastPosition : currentVideoStream->LastSampleTimestamp;
				bool seekForward;
				TimeSpan referenceTime;

				// decide seek direction
				if (isLastSeekForward && position > lastSeekStart && position <= lastSeekActual)
				{
					seekForward = true;
					referenceTime = lastSeekStart + ((position - lastSeekStart) * 0.2);
					DebugMessage(L" - ### Forward seeking continue\n");
				}
				else if (!isLastSeekForward && position < lastSeekStart && position >= lastSeekActual)
				{
					seekForward = false;
					referenceTime = lastSeekStart + ((position - lastSeekStart) * 0.2);
					DebugMessage(L" - ### Backward seeking continue\n");
				}
				else if (position >= playbackPosition)
				{
					seekForward = true;
					referenceTime = playbackPosition + ((position - playbackPosition) * 0.2);
					DebugMessage(L" - ### Forward seeking\n");
				}
				else
				{
					seekForward = false;
					referenceTime = playbackPosition + ((position - playbackPosition) * 0.2);
					DebugMessage(L" - ### Backward seeking\n");
				}

				int64_t min = INT64_MIN;
				int64_t max = INT64_MAX;
				if (seekForward)
				{
					min = stream->ConvertPosition(referenceTime);
				}
				else
				{
					max = stream->ConvertPosition(referenceTime);
				}

				if (avformat_seek_file(avFormatCtx, stream->StreamIndex(), min, seekTarget, max, 0) < 0)
				{
					hr = E_FAIL;
					DebugMessage(L" - ### Error while seeking\n");
				}
				else
				{
					// Flush all active streams
					FlushStreams();

					// get and apply keyframe position for fast seeking
					TimeSpan timestampVideo;
					TimeSpan timestampVideoDuration;
					hr = currentVideoStream->GetNextPacketTimestamp(timestampVideo, timestampVideoDuration);

					while (hr == S_OK && seekForward && timestampVideo < referenceTime)
					{
						// our min position was not respected. try again with higher min and target.
						min += stream->ConvertDuration(TimeSpan{ 50000000 });
						seekTarget += stream->ConvertDuration(TimeSpan{ 50000000 });

						if (avformat_seek_file(avFormatCtx, stream->StreamIndex(), min, seekTarget, max, 0) < 0)
						{
							hr = E_FAIL;
							DebugMessage(L" - ### Error while seeking\n");
						}
						else
						{
							// Flush all active streams
							FlushStreams();

							// get updated timestamp
							hr = currentVideoStream->GetNextPacketTimestamp(timestampVideo, timestampVideoDuration);
						}
					}

					if (hr == S_OK)
					{
						actualPosition = timestampVideo;

						// remember last seek direction
						isLastSeekForward = seekForward;
						lastSeekStart = position;
						lastSeekActual = actualPosition;

						if (currentAudioStream)
						{
							// if we have audio, we need to seek back a bit more to get 100% clean audio
							TimeSpan timestampAudio;
							TimeSpan timestampAudioDuration;
							hr = currentAudioStream->GetNextPacketTimestamp(timestampAudio, timestampAudioDuration);
							if (hr == S_OK)
							{
								// audio stream should start one sample before video
								auto audioTarget = timestampVideo - timestampAudioDuration;
								auto audioPreroll = timestampAudio - timestampVideo;
								if (audioPreroll.count() > 0 && config.FastSeekCleanAudio())
								{
									seekTarget = stream->ConvertPosition(audioTarget - audioPreroll);
									if (av_seek_frame(avFormatCtx, stream->StreamIndex(), seekTarget, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_ANY) < 0)
									{
										hr = E_FAIL;
										DebugMessage(L" - ### Error while seeking\n");
									}
									else
									{
										FlushStreams();

										// Now drop all packets until desired keyframe position
										currentVideoStream->SkipPacketsUntilTimestamp(timestampVideo);
										currentAudioStream->SkipPacketsUntilTimestamp(audioTarget);

										auto sample = currentAudioStream->GetNextSample();
										if (sample)
										{
											actualPosition = sample.Timestamp() + sample.Duration();
										}
									}
								}
								else if (audioPreroll.count() <= 0)
								{
									// Negative audio preroll. Just drop all packets until target position.
									currentAudioStream->SkipPacketsUntilTimestamp(audioTarget);

									hr = currentAudioStream->GetNextPacketTimestamp(timestampAudio, timestampAudioDuration);
									if (hr == S_OK && (config.FastSeekCleanAudio() || (timestampAudio + timestampAudioDuration) <= timestampVideo))
									{
										// decode one audio sample to get clean output
										auto sample = currentAudioStream->GetNextSample();
										if (sample)
										{
											actualPosition = sample.Timestamp() + sample.Duration();
										}
									}
								}
							}
						}
					}
				}
			}
			else
			{
				if (av_seek_frame(avFormatCtx, stream->StreamIndex(), seekTarget, AVSEEK_FLAG_BACKWARD) < 0)
				{
					hr = E_FAIL;
					DebugMessage(L" - ### Error while seeking\n");
				}
				else
				{
					// Flush all active streams
					FlushStreams();
				}
			}
		}
		else
		{
			hr = E_FAIL;
		}

		return hr;
	}

	void FFmpegMediaSource::OnPositionChanged(winrt::Windows::Media::Playback::MediaPlaybackSession const& sender, winrt::Windows::Foundation::IInspectable const& args)
	{
		mutexGuard.lock();
		lastPosition = actualPosition;
		actualPosition = sender.Position();
		mutexGuard.unlock();
	}
}
