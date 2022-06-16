#include "pch.h"
#include "FFmpegMediaSource.h"
#include "LanguageTagConverter.h"
#include "FFmpegMediaSource.g.cpp"
#include "winrt/Windows.ApplicationModel.Core.h"
#include "D3D11VideoSampleProvider.h"
#include "H264AVCSampleProvider.h"
#include "UncompressedAudioSampleProvider.h"
#include "UncompressedFrameProvider.h"
#include "UncompressedSampleProvider.h"
#include "UncompressedVideoSampleProvider.h"
#include "HEVCSampleProvider.h"
#include "NALPacketSampleProvider.h"
#include "CodecChecker.h"
#include "MediaSampleProvider.h"
#include "SubtitleProviderSsaAss.h"
#include "SubtitleProviderBitmap.h"
#include "ChapterInfo.h"
#include "FFmpegReader.h"

using namespace winrt::Windows::Media::MediaProperties;
using namespace FFmpegInteropX;
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

	static int is_hwaccel_pix_fmt(enum AVPixelFormat pix_fmt)
	{
		const AVPixFmtDescriptor* desc = av_pix_fmt_desc_get(pix_fmt);
		return desc->flags & AV_PIX_FMT_FLAG_HWACCEL;
	}

	static AVPixelFormat get_format(struct AVCodecContext* s, const enum AVPixelFormat* fmt)
	{
		AVPixelFormat result_sw = (AVPixelFormat)-1;
		AVPixelFormat result_hw = (AVPixelFormat)-1;
		AVPixelFormat format;
		int index = 0;
		do
		{
			format = fmt[index++];

			//		
			if (format != -1)
			{
				if (s->hw_device_ctx && format == AV_PIX_FMT_D3D11)
				{
					// we only support D3D11 HW format (not D3D11_VLD)
					result_hw = format;
				}
				else if (result_sw == -1 && !is_hwaccel_pix_fmt(format))
				{
					// take first non hw accelerated format
					result_sw = format;
				}
				else if (format == AV_PIX_FMT_NV12 && result_sw != AV_PIX_FMT_YUVA420P)
				{
					// switch SW format to NV12 if available, unless this is an alpha channel file
					result_sw = format;
				}
			}
		} while (format != -1);


		if (result_hw != -1)
		{
			return result_hw;
		}
		else
		{
			return result_sw;
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


	HRESULT FFmpegMediaSource::InitFFmpegContext()
	{
		HRESULT hr = S_OK;

		if (SUCCEEDED(hr))
		{
			if (avformat_find_stream_info(avFormatCtx, NULL) < 0)
			{
				hr = E_FAIL; // Error finding info
			}
		}

		if (SUCCEEDED(hr))
		{
			m_pReader = std::shared_ptr<FFmpegInteropX::FFmpegReader>(new FFmpegInteropX::FFmpegReader(avFormatCtx, sampleProviders));
			if (m_pReader == nullptr)
			{
				hr = E_OUTOFMEMORY;
			}
		}
		auto implConfig = config.as<implementation::MediaSourceConfig>();
		// do not use start time for pure subtitle files
		if (implConfig->IsExternalSubtitleParser && avFormatCtx->nb_streams == 1)
		{
			avFormatCtx->start_time = AV_NOPTS_VALUE;
		}

		const AVCodec* avVideoCodec;
		auto videoStreamIndex = av_find_best_stream(avFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &avVideoCodec, 0);
		auto audioStreamIndex = av_find_best_stream(avFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
		auto subtitleStreamIndex = av_find_best_stream(avFormatCtx, AVMEDIA_TYPE_SUBTITLE, -1, -1, NULL, 0);

		attachedFileHelper = shared_ptr<AttachedFileHelper>(new AttachedFileHelper(config));

		// first parse attached files, so they are available for subtitle streams during initialize
		if (config.UseEmbeddedSubtitleFonts())
		{
			for (unsigned int index = 0; index < avFormatCtx->nb_streams; index++)
			{
				auto avStream = avFormatCtx->streams[index];
				if (avStream->codecpar->codec_type == AVMEDIA_TYPE_ATTACHMENT)
				{
					auto fileName = av_dict_get(avStream->metadata, "filename", NULL, 0);
					auto mimetype = av_dict_get(avStream->metadata, "mimetype", NULL, 0);
					if (fileName && avStream->codecpar->extradata && avStream->codecpar->extradata_size > 0)
					{
						auto name = StringUtils::Utf8ToPlatformString(fileName->value);
						auto mime = mimetype ? StringUtils::Utf8ToPlatformString(mimetype->value) : L"";

						auto file = winrt::make_self<AttachedFile>(name, mime, avStream);
						attachedFileHelper->AddAttachedFile(file.as<winrt::FFmpegInteropXWinUI::AttachedFile>());
					}
				}
			}
		}

		for (unsigned int index = 0; index < avFormatCtx->nb_streams; index++)
		{
			auto avStream = avFormatCtx->streams[index];
			avStream->discard = AVDISCARD_ALL; // all streams are disabled until we enable them

			std::shared_ptr<MediaSampleProvider> stream = nullptr;

			if (avStream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO && !implConfig->IsFrameGrabber && !implConfig->IsExternalSubtitleParser)
			{
				stream = CreateAudioStream(avStream, index);
				if (stream)
				{
					if (index == audioStreamIndex)
					{
						stream->AudioInfo().as<implementation::AudioStreamInfo>()->SetDefault();
						currentAudioStream = stream;
						audioStrInfos.InsertAt(0, stream->AudioInfo());
						audioStreams.insert(audioStreams.begin(), stream);
					}
					else
					{
						audioStrInfos.Append(stream->AudioInfo());
						audioStreams.push_back(stream);
					}
				}
			}
			else if (avStream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO && avStream->disposition == AV_DISPOSITION_ATTACHED_PIC && thumbnailStreamIndex == AVERROR_STREAM_NOT_FOUND && !implConfig->IsExternalSubtitleParser)
			{
				thumbnailStreamIndex = index;
			}
			else if (avStream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO && !implConfig->IsExternalSubtitleParser)
			{
				stream = CreateVideoStream(avStream, index);
				if (stream)
				{
					if (index == videoStreamIndex)
					{
						stream->VideoInfo().as<implementation::VideoStreamInfo>()->isDefault = true;
						currentVideoStream = stream;
						videoStreams.insert(videoStreams.begin(), stream);
						videoStrInfos.InsertAt(0, stream->VideoInfo());
					}
					else
					{
						videoStreams.push_back(stream);
						videoStrInfos.Append(stream->VideoInfo());
					}
				}
			}
			else if (avStream->codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE)
			{
				stream = CreateSubtitleSampleProvider(avStream, index);
				if (stream)
				{
					if (index == subtitleStreamIndex)
					{
						stream->SubtitleInfo().as<implementation::SubtitleStreamInfo>()->SetDefault();
						subtitleStrInfos.InsertAt(0, stream->SubtitleInfo());
						subtitleStreams.insert(subtitleStreams.begin(), (std::reinterpret_pointer_cast<SubtitleProvider>(stream)));
					}
					else
					{
						subtitleStrInfos.Append(stream->SubtitleInfo());
						subtitleStreams.push_back((std::reinterpret_pointer_cast<SubtitleProvider>(stream)));
					}

					// enable all subtitle streams for external subtitle parsing
					if (implConfig->IsExternalSubtitleParser)
					{
						(std::reinterpret_pointer_cast<SubtitleProvider>(stream))->EnableStream();
					}
				}
			}

			sampleProviders.push_back(stream);
		}

		if (!currentAudioStream && audioStreams.size() > 0)
		{
			currentAudioStream = audioStreams[0];
		}
		if (!currentVideoStream && videoStreams.size() > 0)
		{
			currentVideoStream = videoStreams[0];
		}

		audioStreamInfos = audioStrInfos.GetView();
		subtitleStreamInfos = subtitleStrInfos.GetView();
		videoStreamInfos = videoStrInfos.GetView();

		if (!config.FFmpegVideoFilters().empty())
		{
			SetFFmpegVideoFilters(config.FFmpegVideoFilters());
		}

		if (!config.FFmpegAudioFilters().empty())
		{
			SetFFmpegAudioFilters(config.FFmpegAudioFilters());
		}

		if (currentVideoStream)
		{
			auto videoDescriptor = (currentVideoStream->StreamDescriptor().as<VideoStreamDescriptor>());
			auto pixelAspect = (double)videoDescriptor.EncodingProperties().PixelAspectRatio().Numerator() / videoDescriptor.EncodingProperties().PixelAspectRatio().Denominator();
			auto videoAspect = ((double)currentVideoStream->m_pAvCodecCtx->width / currentVideoStream->m_pAvCodecCtx->height) / pixelAspect;
			for each (auto stream in subtitleStreams)
			{
				stream->NotifyVideoFrameSize(currentVideoStream->m_pAvCodecCtx->width, currentVideoStream->m_pAvCodecCtx->height, videoAspect);
			}
		}

		if (currentVideoStream && currentAudioStream)
		{
			mss = MediaStreamSource(currentVideoStream->StreamDescriptor(), currentAudioStream->StreamDescriptor());
			currentVideoStream->EnableStream();
			currentAudioStream->EnableStream();
		}
		else if (currentAudioStream)
		{
			mss = MediaStreamSource(currentAudioStream->StreamDescriptor());
			currentAudioStream->EnableStream();
		}
		else if (currentVideoStream)
		{
			mss = MediaStreamSource(currentVideoStream->StreamDescriptor());
			currentVideoStream->EnableStream();
		}
		else if (subtitleStreams.size() == 0 || !implConfig->IsExternalSubtitleParser)
		{
			//only fail if there are no media streams (audio, video, or subtitle)
			hr = E_FAIL;
		}
		//if the streams are subtitles only, there will be no media stream source
		if (mss != nullptr) {
			if (SUCCEEDED(hr))
			{
				for each (auto stream in audioStreams)
				{
					if (stream != currentAudioStream)
					{
						mss.AddStreamDescriptor(stream->StreamDescriptor());
					}
				}

				for each (auto stream in videoStreams)
				{
					if (stream != currentVideoStream)
					{
						mss.AddStreamDescriptor(stream->StreamDescriptor());
					}
				}

				auto chapters = winrt::single_threaded_vector<winrt::FFmpegInteropXWinUI::ChapterInfo>();
				if (avFormatCtx->chapters && avFormatCtx->nb_chapters > 1)
				{
					for (size_t i = 0; i < avFormatCtx->nb_chapters; i++)
					{
						auto chapter = avFormatCtx->chapters[i];
						auto entry = av_dict_get(chapter->metadata, "title", NULL, AV_DICT_IGNORE_SUFFIX);
						if (entry)
						{
							auto title = StringUtils::Utf8ToPlatformString(entry->value);
							TimeSpan start{ (long long)((chapter->start / (double)chapter->time_base.den) * chapter->time_base.num * 10000000) };
							TimeSpan duration{ (long long)(((chapter->end - chapter->start) / (double)chapter->time_base.den) * chapter->time_base.num * 10000000) };

							// compensate for start time offset
							if (avFormatCtx->start_time != AV_NOPTS_VALUE)
							{
								start = TimeSpan(start.count() - (avFormatCtx->start_time * 10));
							}

							// cut off negative start times
							if (start.count() < 0)
							{
								duration = TimeSpan(duration.count() + start.count());
								start = TimeSpan(0);
							}

							if (duration.count() > 0)
							{
								auto chapInfo = winrt::FFmpegInteropXWinUI::ChapterInfo(title, start, duration);
								chapters.Append(chapInfo);
							}
						}
					}
				}
				chapterInfos = chapters.GetView();
			}

			if (SUCCEEDED(hr))
			{
				// Convert media duration from AV_TIME_BASE to TimeSpan unit
				mediaDuration = TimeSpan(LONGLONG(avFormatCtx->duration * 10000000 / double(AV_TIME_BASE)));

				// Assign initial BufferTime to MediaStreamSource
				mss.BufferTime(fileStreamData ? config.DefaultBufferTime() : config.DefaultBufferTimeUri());

				if (winrt::Windows::Foundation::Metadata::ApiInformation::IsPropertyPresent(L"Windows.Media.Core.MediaStreamSource", L"MaxSupportedPlaybackRate"))
				{
					mss.MaxSupportedPlaybackRate(config.MaxSupportedPlaybackRate());
				}

				if (mediaDuration.count() > 0)
				{
					mss.Duration(mediaDuration);
					mss.CanSeek(true);
				}

				auto title = av_dict_get(avFormatCtx->metadata, "title", NULL, 0);
				auto titleStr = title ? StringUtils::Utf8ToPlatformString(title->value) : L"";
				auto codecStr = StringUtils::Utf8ToPlatformString(avFormatCtx->iformat->name);
				formatInfo = winrt::FFmpegInteropXWinUI::FormatInfo(titleStr, codecStr, mediaDuration, avFormatCtx->bit_rate);

				startingRequestedToken = mss.Starting(TypedEventHandler<MediaStreamSource, MediaStreamSourceStartingEventArgs>(this, &FFmpegMediaSource::OnStarting));
				sampleRequestedToken = mss.SampleRequested(TypedEventHandler<MediaStreamSource, MediaStreamSourceSampleRequestedEventArgs>(this, &FFmpegMediaSource::OnSampleRequested));
				switchStreamRequestedToken = mss.SwitchStreamsRequested(TypedEventHandler<MediaStreamSource, MediaStreamSourceSwitchStreamsRequestedEventArgs>(this, &FFmpegMediaSource::OnSwitchStreamsRequested));
			}
		}

		return hr;
	}


	std::shared_ptr<SubtitleProvider> FFmpegMediaSource::CreateSubtitleSampleProvider(AVStream* avStream, int index)
	{
		HRESULT hr = S_OK;
		std::shared_ptr<SubtitleProvider> avSubsStream = nullptr;
		auto avSubsCodec = avcodec_find_decoder(avStream->codecpar->codec_id);
		if (avSubsCodec)
		{
			// allocate a new decoding context
			auto avSubsCodecCtx = avcodec_alloc_context3(avSubsCodec);
			if (!avSubsCodecCtx)
			{
				DebugMessage(L"Could not allocate a decoding context\n");
				hr = E_OUTOFMEMORY;
			}

			//inject custom properties
			if (config.AutoCorrectAnsiSubtitles() && config.as<implementation::MediaSourceConfig>()->IsExternalSubtitleParser && streamByteOrderMark != ByteOrderMark::UTF8)
			{
				hstring key = config.AnsiSubtitleEncoding().Name();
				std::string keyA = StringUtils::PlatformStringToUtf8String(key);
				const char* keyChar = keyA.c_str();

				if (av_opt_set(avSubsCodecCtx, "sub_charenc", keyChar, AV_OPT_SEARCH_CHILDREN) < 0)
				{
					DebugMessage(L"Could not set sub_charenc on subtitle provider\n");
				}
				if (av_opt_set_int(avSubsCodecCtx, "sub_charenc_mode", FF_SUB_CHARENC_MODE_AUTOMATIC, AV_OPT_SEARCH_CHILDREN) < 0)
				{
					DebugMessage(L"Could not set sub_charenc_mode on subtitle provider\n");
				}
			}

			if (SUCCEEDED(hr))
			{
				// initialize the stream parameters with demuxer information
				if (avcodec_parameters_to_context(avSubsCodecCtx, avStream->codecpar) < 0)
				{
					hr = E_FAIL;
				}

				if (SUCCEEDED(hr))
				{
					if (avcodec_open2(avSubsCodecCtx, avSubsCodec, NULL) < 0)
					{
						hr = E_FAIL;
					}
					else
					{
						if ((avSubsCodecCtx->codec_descriptor->props & AV_CODEC_PROP_TEXT_SUB) == AV_CODEC_PROP_TEXT_SUB)
						{
							avSubsStream = std::shared_ptr<SubtitleProvider>(new SubtitleProviderSsaAss(m_pReader, avFormatCtx, avSubsCodecCtx, config, index, dispatcher, attachedFileHelper));
						}
						else if ((avSubsCodecCtx->codec_descriptor->props & AV_CODEC_PROP_BITMAP_SUB) == AV_CODEC_PROP_BITMAP_SUB)
						{
							if (winrt::Windows::Foundation::Metadata::ApiInformation::IsEnumNamedValuePresent(L"Windows.Media.Core.TimedMetadataKind", L"ImageSubtitle"))
							{
								avSubsStream = std::shared_ptr<SubtitleProvider>(new SubtitleProviderBitmap(m_pReader, avFormatCtx, avSubsCodecCtx, config, index, dispatcher));
							}
						}
						else
						{
							hr = E_FAIL;
						}
					}
				}
			}

			if (SUCCEEDED(hr))
			{
				avSubsStream->SubtitleDelay = SubtitleDelay();
				hr = avSubsStream->Initialize();
			}

			if (FAILED(hr))
			{
				avSubsStream = nullptr;
			}

			// free codec context if failed
			if (!avSubsStream && avSubsCodecCtx)
			{
				avcodec_free_context(&avSubsCodecCtx);
			}
		}
		else
		{
			DebugMessage(L"Could not find decoder\n");
		}

		return avSubsStream;
	}


	std::shared_ptr<MediaSampleProvider> FFmpegMediaSource::CreateAudioStream(AVStream* avStream, int index)
	{
		HRESULT hr = S_OK;
		std::shared_ptr<MediaSampleProvider> audioStream = nullptr;
		auto avAudioCodec = avcodec_find_decoder(avStream->codecpar->codec_id);
		if (avAudioCodec)
		{
			// allocate a new decoding context
			auto avAudioCodecCtx = avcodec_alloc_context3(avAudioCodec);
			if (!avAudioCodecCtx)
			{
				DebugMessage(L"Could not allocate a decoding context\n");
				hr = E_OUTOFMEMORY;
			}

			if (SUCCEEDED(hr))
			{
				// initialize the stream parameters with demuxer information
				if (avcodec_parameters_to_context(avAudioCodecCtx, avStream->codecpar) < 0)
				{
					hr = E_FAIL;
				}

				if (SUCCEEDED(hr))
				{
					if (avAudioCodecCtx->sample_fmt == AV_SAMPLE_FMT_S16P)
					{
						avAudioCodecCtx->request_sample_fmt = AV_SAMPLE_FMT_S16;
					}
					else if (avAudioCodecCtx->sample_fmt == AV_SAMPLE_FMT_S32P)
					{
						avAudioCodecCtx->request_sample_fmt = AV_SAMPLE_FMT_S32;
					}
					else if (avAudioCodecCtx->sample_fmt == AV_SAMPLE_FMT_FLTP)
					{
						avAudioCodecCtx->request_sample_fmt = AV_SAMPLE_FMT_FLT;
					}

					// enable multi threading
					unsigned threads = std::thread::hardware_concurrency();
					if (threads > 0)
					{
						avAudioCodecCtx->thread_count = config.MaxAudioThreads() == 0 ? threads : min(threads, config.MaxAudioThreads());
						avAudioCodecCtx->thread_type = FF_THREAD_FRAME | FF_THREAD_SLICE;
					}

					if (avcodec_open2(avAudioCodecCtx, avAudioCodec, NULL) < 0)
					{
						hr = E_FAIL;
					}
					else if (avAudioCodecCtx->sample_fmt == AV_SAMPLE_FMT_NONE)
					{
						hr = E_FAIL;
					}
					else
					{
						// Detect audio format and create audio stream descriptor accordingly
						audioStream = CreateAudioSampleProvider(avStream, avAudioCodecCtx, index);
					}
				}
			}

			// free codec context if failed
			if (!audioStream && avAudioCodecCtx)
			{
				avcodec_free_context(&avAudioCodecCtx);
			}
		}
		else
		{
			DebugMessage(L"Could not find decoder\n");
		}

		return audioStream;
	}


	std::shared_ptr<MediaSampleProvider> FFmpegMediaSource::CreateVideoStream(AVStream* avStream, int index)
	{
		HRESULT hr = S_OK;
		std::shared_ptr<MediaSampleProvider> result = nullptr;

		// Find the video stream and its decoder
		auto avVideoCodec = avcodec_find_decoder(avStream->codecpar->codec_id);

		if (avVideoCodec)
		{
			auto tryAv1hw = avVideoCodec->id == AVCodecID::AV_CODEC_ID_AV1 && avVideoCodec->name != "av1" && config.VideoDecoderMode() == VideoDecoderMode::Automatic;
			auto libdav1d = tryAv1hw ? avVideoCodec : NULL;
			if (tryAv1hw)
			{
				avVideoCodec = avcodec_find_decoder_by_name("av1");
				if (!avVideoCodec)
				{
					avVideoCodec = libdav1d;
				}
			}

			// allocate a new decoding context
			auto avVideoCodecCtx = avcodec_alloc_context3(avVideoCodec);
			if (!avVideoCodecCtx)
			{
				DebugMessage(L"Could not allocate a decoding context\n");
				hr = E_OUTOFMEMORY;
			}

			// create and assign HW device context, if supported and requested
			if (SUCCEEDED(hr) && config.VideoDecoderMode() == VideoDecoderMode::Automatic)
			{
				int i = 0;
				while (SUCCEEDED(hr))
				{
					auto config = avcodec_get_hw_config(avVideoCodec, i++);
					if (config)
					{
						if (config->pix_fmt == AV_PIX_FMT_D3D11 && config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX)
						{
							AVBufferRef* hwContext = NULL;
							if (!avHardwareContext)
							{
								avHardwareContext = av_hwdevice_ctx_alloc(AVHWDeviceType::AV_HWDEVICE_TYPE_D3D11VA);
							}

							if (!avHardwareContext)
							{
								hr = E_FAIL;
							}

							if (avVideoCodecCtx->codec_id == AV_CODEC_ID_VC1 || avVideoCodecCtx->codec_id == AV_CODEC_ID_WMV3)
							{
								// workaround for VC1 and WMV3: use default device context, later replace with actual MSS device context
								if (!avHardwareContextDefault)
								{
									if (SUCCEEDED(hr))
									{
										hr = av_hwdevice_ctx_create(&avHardwareContextDefault, AVHWDeviceType::AV_HWDEVICE_TYPE_D3D11VA, NULL, NULL, 0);
									}

									if (SUCCEEDED(hr))
									{
										hr = av_hwdevice_ctx_init(avHardwareContextDefault);
									}

									if (FAILED(hr) && avHardwareContextDefault)
									{
										av_buffer_unref(&avHardwareContextDefault);
									}
								}

								if (SUCCEEDED(hr))
								{
									hwContext = avHardwareContextDefault;
								}
							}
							else
							{
								if (SUCCEEDED(hr))
								{
									hwContext = avHardwareContext;
								}
							}

							if (hwContext)
							{
								avVideoCodecCtx->hw_device_ctx = av_buffer_ref(hwContext);
							}
							else
							{
								hr = E_OUTOFMEMORY;
							}
							break;
						}
					}
					else
					{
						break;
					}
				}
			}

			if (SUCCEEDED(hr))
			{
				avVideoCodecCtx->get_format = &get_format;

				// initialize the stream parameters with demuxer information
				hr = avcodec_parameters_to_context(avVideoCodecCtx, avStream->codecpar);
			}

			if (SUCCEEDED(hr))
			{
				// enable multi threading only for SW decoders
				if (!avVideoCodecCtx->hw_device_ctx)
				{
					unsigned threads = std::thread::hardware_concurrency();
					avVideoCodecCtx->thread_count = config.MaxVideoThreads() == 0 ? threads : min(threads, config.MaxVideoThreads());
					avVideoCodecCtx->thread_type = config.as<implementation::MediaSourceConfig>()->IsFrameGrabber ? FF_THREAD_SLICE : FF_THREAD_FRAME | FF_THREAD_SLICE;
				}

				hr = avcodec_open2(avVideoCodecCtx, avVideoCodec, NULL);
			}

			if (SUCCEEDED(hr))
			{
				// Detect video format and create video stream descriptor accordingly
				result = CreateVideoSampleProvider(avStream, avVideoCodecCtx, index);
			}

			// free codec context if failed
			if (!result && avVideoCodecCtx)
			{
				avcodec_free_context(&avVideoCodecCtx);
			}
		}

		return result;
	}



	void FFmpegMediaSource::SetSubtitleDelay(Windows::Foundation::TimeSpan const& delay)
	{
		mutexGuard.lock();
		try
		{
			for each (auto subtitleStream in subtitleStreams)
			{
				subtitleStream->SetSubtitleDelay(delay);
			}

			subtitleDelay = delay;
		}
		catch (...)
		{
		}
		mutexGuard.unlock();
	}

	void FFmpegMediaSource::SetFFmpegAudioFilters(hstring const& audioFilters)
	{
		mutexGuard.lock();
		if (currentAudioStream)
		{
			currentAudioStream->SetFilters(audioFilters);
			currentAudioEffects = audioFilters;
		}
		mutexGuard.unlock();
	}

	void FFmpegMediaSource::SetFFmpegVideoFilters(hstring const& videoEffects)
	{
		mutexGuard.lock();
		if (currentVideoStream)
		{
			currentVideoStream->SetFilters(videoEffects);
			//TODO store and apply video effects on video stream change!
			//currentVideoEffects = videoEffects;
		}
		mutexGuard.unlock();
	}

	void FFmpegMediaSource::DisableAudioEffects()
	{
		mutexGuard.lock();
		if (currentAudioStream)
		{
			currentAudioStream->DisableFilters();
			currentAudioEffects = hstring();
		}
		mutexGuard.unlock();
	}

	void FFmpegMediaSource::DisableVideoEffects()
	{
		mutexGuard.lock();
		if (currentVideoStream)
		{
			currentVideoStream->DisableFilters();
		}
		mutexGuard.unlock();
	}

	FFmpegInteropXWinUI::MediaThumbnailData FFmpegMediaSource::ExtractThumbnail()
	{
		if (thumbnailStreamIndex != AVERROR_STREAM_NOT_FOUND)
		{
			// FFmpeg identifies album/cover art from a music file as a video stream
			// Avoid creating unnecessarily video stream from this album/cover art
			if (avFormatCtx->streams[thumbnailStreamIndex]->disposition == AV_DISPOSITION_ATTACHED_PIC)
			{
				auto imageStream = avFormatCtx->streams[thumbnailStreamIndex];
				//save album art to file.
				hstring extension = L".jpeg";
				switch (imageStream->codecpar->codec_id)
				{
				case AV_CODEC_ID_MJPEG:
				case AV_CODEC_ID_MJPEGB:
				case AV_CODEC_ID_JPEG2000:
				case AV_CODEC_ID_JPEGLS: extension = L".jpeg"; break;
				case AV_CODEC_ID_PNG: extension = L".png"; break;
				case AV_CODEC_ID_BMP: extension = L".bmp"; break;
				}

				auto vector = std::vector<uint8_t>(imageStream->attached_pic.data, imageStream->attached_pic.data + imageStream->attached_pic.size);
				DataWriter writer = DataWriter();
				writer.WriteBytes(vector);

				auto retValue = MediaThumbnailData(writer.DetachBuffer(), extension);
				return retValue;
			}
		}

		return nullptr;
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

	std::shared_ptr<MediaSampleProvider> FFmpegMediaSource::CreateAudioSampleProvider(AVStream* avStream, AVCodecContext* avAudioCodecCtx, int index)
	{
		std::shared_ptr<MediaSampleProvider> audioSampleProvider = nullptr;
		if (avAudioCodecCtx->codec_id == AV_CODEC_ID_AAC && config.PassthroughAudioAAC())
		{
			AudioEncodingProperties encodingProperties;
			if (avAudioCodecCtx->extradata_size == 0)
			{
				encodingProperties = AudioEncodingProperties::CreateAacAdts(avAudioCodecCtx->sample_rate, avAudioCodecCtx->channels, (unsigned int)avAudioCodecCtx->bit_rate);
			}
			else
			{
				encodingProperties = AudioEncodingProperties::CreateAac(avAudioCodecCtx->profile == FF_PROFILE_AAC_HE || avAudioCodecCtx->profile == FF_PROFILE_AAC_HE_V2 ? avAudioCodecCtx->sample_rate / 2 : avAudioCodecCtx->sample_rate, avAudioCodecCtx->channels, (unsigned int)avAudioCodecCtx->bit_rate);
			}
			audioSampleProvider = std::shared_ptr<MediaSampleProvider>(new CompressedSampleProvider(m_pReader, avFormatCtx, avAudioCodecCtx, config, index, encodingProperties, HardwareDecoderStatus::Unknown));
		}
		else if (avAudioCodecCtx->codec_id == AV_CODEC_ID_MP3 && config.PassthroughAudioMP3())
		{
			AudioEncodingProperties encodingProperties = AudioEncodingProperties::CreateMp3(avAudioCodecCtx->sample_rate, avAudioCodecCtx->channels, (unsigned int)avAudioCodecCtx->bit_rate);
			audioSampleProvider = std::shared_ptr<MediaSampleProvider>(new CompressedSampleProvider(m_pReader, avFormatCtx, avAudioCodecCtx, config, index, encodingProperties, HardwareDecoderStatus::Unknown));
		}
		else
		{
			audioSampleProvider = std::shared_ptr<MediaSampleProvider>(new UncompressedAudioSampleProvider(m_pReader, avFormatCtx, avAudioCodecCtx, config, index));
		}

		auto hr = audioSampleProvider->Initialize();
		if (FAILED(hr))
		{
			audioSampleProvider = nullptr;
		}

		return audioSampleProvider;
	}

	bool FFmpegMediaSource::CheckUseHardwareAcceleration(AVCodecContext* avCodecCtx, HardwareAccelerationStatus status, HardwareDecoderStatus& hardwareDecoderStatus, int maxProfile, int maxLevel)
	{
		bool result = false;
		if (!config.as<implementation::MediaSourceConfig>()->IsFrameGrabber)
		{
#pragma warning (disable: 4973)

			if (config.VideoDecoderMode() == VideoDecoderMode::AutomaticSystemDecoder)
			{
				result = CodecChecker::CheckUseHardwareAcceleration(status,
					avCodecCtx->codec_id, avCodecCtx->profile, avCodecCtx->width, avCodecCtx->height);

				// check level, if restricted
				if (result && maxLevel >= 0)
				{
					result = avCodecCtx->level <= maxLevel;
				}

				hardwareDecoderStatus = result ? HardwareDecoderStatus::Available : HardwareDecoderStatus::NotAvailable;
			}
			else if (config.VideoDecoderMode() == VideoDecoderMode::ForceSystemDecoder)
			{
				result = true;
			}
			else
			{
				result = false;
			}
#pragma warning (default: 4973)

		}

		return result;
	}

	std::shared_ptr<MediaSampleProvider> FFmpegMediaSource::CreateVideoSampleProvider(AVStream* avStream, AVCodecContext* avVideoCodecCtx, int index)
	{
		std::shared_ptr<MediaSampleProvider> videoSampleProvider = nullptr;
		std::shared_ptr<VideoEncodingProperties> videoProperties = nullptr;
		winrt::FFmpegInteropXWinUI::HardwareDecoderStatus hardwareDecoderStatus;

#pragma warning (disable: 4973)

		if (config.VideoDecoderMode() == VideoDecoderMode::AutomaticSystemDecoder)
		{
			CodecChecker::Initialize();
		}

		if (avVideoCodecCtx->codec_id == AV_CODEC_ID_H264 &&
			(CheckUseHardwareAcceleration(avVideoCodecCtx, CodecChecker::HardwareAccelerationH264(), hardwareDecoderStatus, config.SystemDecoderH264MaxProfile(), config.SystemDecoderH264MaxLevel())))
		{
			auto videoProperties = VideoEncodingProperties::CreateH264();

			// Check for H264 bitstream flavor. H.264 AVC extradata starts with 1 while non AVC one starts with 0
			if (avVideoCodecCtx->extradata != nullptr && avVideoCodecCtx->extradata_size > 0 && avVideoCodecCtx->extradata[0] == 1)
			{
				videoSampleProvider = std::shared_ptr<MediaSampleProvider>(new H264AVCSampleProvider(m_pReader, avFormatCtx, avVideoCodecCtx, config, index, videoProperties, hardwareDecoderStatus));
			}
			else
			{
				videoSampleProvider = std::shared_ptr<MediaSampleProvider>(new NALPacketSampleProvider(m_pReader, avFormatCtx, avVideoCodecCtx, config, index, videoProperties, hardwareDecoderStatus));
			}
		}
		else if (avVideoCodecCtx->codec_id == AV_CODEC_ID_HEVC &&
			CheckUseHardwareAcceleration(avVideoCodecCtx, CodecChecker::HardwareAccelerationHEVC(), hardwareDecoderStatus, config.SystemDecoderHEVCMaxProfile(), config.SystemDecoderHEVCMaxLevel()) &&
			winrt::Windows::Foundation::Metadata::ApiInformation::IsMethodPresent(L"Windows.Media.MediaProperties.VideoEncodingProperties", L"CreateHevc"))
		{
			auto videoProperties = VideoEncodingProperties::CreateHevc();

			// Check for HEVC bitstream flavor.
			if (avVideoCodecCtx->extradata != nullptr && avVideoCodecCtx->extradata_size > 22 &&
				(avVideoCodecCtx->extradata[0] || avVideoCodecCtx->extradata[1] || avVideoCodecCtx->extradata[2] > 1))
			{
				videoSampleProvider = std::shared_ptr<MediaSampleProvider>(new HEVCSampleProvider(m_pReader, avFormatCtx, avVideoCodecCtx, config, index, videoProperties, hardwareDecoderStatus));
			}
			else
			{
				videoSampleProvider = std::shared_ptr<MediaSampleProvider>(new NALPacketSampleProvider(m_pReader, avFormatCtx, avVideoCodecCtx, config, index, videoProperties, hardwareDecoderStatus));
			}
		}
		else if (avVideoCodecCtx->codec_id == AV_CODEC_ID_WMV3 &&
			CheckUseHardwareAcceleration(avVideoCodecCtx, CodecChecker::HardwareAccelerationWMV3(), hardwareDecoderStatus, -1, -1) &&
			avVideoCodecCtx->extradata_size > 0)
		{
			auto videoProperties = VideoEncodingProperties();
			videoProperties.Subtype(MediaEncodingSubtypes::Wmv3());
			auto extradata = std::vector<uint8_t>((uint8_t)&avVideoCodecCtx->extradata, (uint8_t)&avVideoCodecCtx->extradata + avVideoCodecCtx->extradata_size);

			videoProperties.SetFormatUserData(extradata);
			videoSampleProvider = std::shared_ptr<MediaSampleProvider>(new CompressedSampleProvider(m_pReader, avFormatCtx, avVideoCodecCtx, config, index, videoProperties, hardwareDecoderStatus));
		}
		else if (avVideoCodecCtx->codec_id == AV_CODEC_ID_VC1 &&
			CheckUseHardwareAcceleration(avVideoCodecCtx, CodecChecker::HardwareAccelerationVC1(), hardwareDecoderStatus, -1, -1) &&
			avVideoCodecCtx->extradata_size > 0)
		{
			auto videoProperties = VideoEncodingProperties();
			videoProperties.Subtype(MediaEncodingSubtypes::Wvc1());

			auto extradata = std::vector<uint8_t>((uint8_t)&avVideoCodecCtx->extradata, (uint8_t)&avVideoCodecCtx->extradata + avVideoCodecCtx->extradata_size);

			//auto extradata = Platform::ArrayReference<uint8_t>(avVideoCodecCtx->extradata, avVideoCodecCtx->extradata_size);
			videoProperties.SetFormatUserData(extradata);
			videoSampleProvider = std::shared_ptr<MediaSampleProvider>(new CompressedSampleProvider(m_pReader, avFormatCtx, avVideoCodecCtx, config, index, videoProperties, hardwareDecoderStatus));
		}
		else if (avVideoCodecCtx->codec_id == AV_CODEC_ID_MPEG2VIDEO &&
			CheckUseHardwareAcceleration(avVideoCodecCtx, CodecChecker::HardwareAccelerationMPEG2(), hardwareDecoderStatus, -1, -1))
		{
			auto videoProperties = VideoEncodingProperties();
			videoProperties.Subtype(MediaEncodingSubtypes::Mpeg2());

			videoSampleProvider = std::shared_ptr<MediaSampleProvider>(new CompressedSampleProvider(m_pReader, avFormatCtx, avVideoCodecCtx, config, index, videoProperties, hardwareDecoderStatus));
		}
		else if (avVideoCodecCtx->codec_id == AV_CODEC_ID_VP9 &&
			CheckUseHardwareAcceleration(avVideoCodecCtx, CodecChecker::HardwareAccelerationVP9(), hardwareDecoderStatus, -1, -1) &&
			winrt::Windows::Foundation::Metadata::ApiInformation::IsPropertyPresent(L"Windows.Media.MediaProperties.MediaEncodingSubtypes", L"Vp9"))
		{
			auto videoProperties = VideoEncodingProperties();
			videoProperties.Subtype(MediaEncodingSubtypes::Vp9());

			videoSampleProvider = std::shared_ptr<MediaSampleProvider>(new CompressedSampleProvider(m_pReader, avFormatCtx, avVideoCodecCtx, config, index, videoProperties, hardwareDecoderStatus));
		}
		else if (avVideoCodecCtx->codec_id == AV_CODEC_ID_VP8 &&
			CheckUseHardwareAcceleration(avVideoCodecCtx, CodecChecker::HardwareAccelerationVP8(), hardwareDecoderStatus, -1, -1) &&
			winrt::Windows::Foundation::Metadata::ApiInformation::IsTypePresent(L"Windows.Media.Core.CodecSubtypes"))

		{
			auto videoProperties = VideoEncodingProperties();
			videoProperties.Subtype(Windows::Media::Core::CodecSubtypes::VideoFormatVP80());

			videoSampleProvider = std::shared_ptr<MediaSampleProvider>(new CompressedSampleProvider(m_pReader, avFormatCtx, avVideoCodecCtx, config, index, videoProperties, hardwareDecoderStatus));
		}
		else if (avVideoCodecCtx->hw_device_ctx)
		{
			hardwareDecoderStatus = HardwareDecoderStatus::Available;
			videoSampleProvider = std::shared_ptr<MediaSampleProvider>(new D3D11VideoSampleProvider(m_pReader, avFormatCtx, avVideoCodecCtx, config, index, hardwareDecoderStatus));
		}
		else
		{
			if (config.VideoDecoderMode() == VideoDecoderMode::AutomaticSystemDecoder)
			{
				hardwareDecoderStatus = HardwareDecoderStatus::NotAvailable;
			}
			videoSampleProvider = std::shared_ptr<MediaSampleProvider>(new UncompressedVideoSampleProvider(m_pReader, avFormatCtx, avVideoCodecCtx, config, index, hardwareDecoderStatus));
		}

#pragma warning (default: 4973)

		auto hr = videoSampleProvider->Initialize();

		if (FAILED(hr))
		{
			videoSampleProvider = nullptr;
		}

		return videoSampleProvider;
	}

	HRESULT FFmpegMediaSource::ParseOptions(PropertySet const& ffmpegOptions)
	{
		HRESULT hr = S_OK;

		// Convert FFmpeg options given in PropertySet to AVDictionary. List of options can be found in https://www.ffmpeg.org/ffmpeg-protocols.html
		if (ffmpegOptions != nullptr)
		{
			auto options = ffmpegOptions.First();

			while (options.HasCurrent())
			{
				auto key = StringUtils::PlatformStringToUtf8String(options.Current().Key());
				auto stringValue = options.Current().Value().try_as<winrt::Windows::Foundation::IStringable>();
				if (stringValue)
				{
					auto value = StringUtils::PlatformStringToUtf8String(stringValue.ToString());

					// Add key and value pair entry
					if (av_dict_set(&avDict, key.c_str(), value.c_str(), 0) < 0)
					{
						hr = E_INVALIDARG;
						break;
					}
				}
				options.MoveNext();
			}
		}

		return hr;
	}

	void FFmpegMediaSource::OnStarting(MediaStreamSource const& sender, MediaStreamSourceStartingEventArgs const& args)
	{
		mutexGuard.lock();
		MediaStreamSourceStartingRequest request = args.Request();

		if (isFirstSeek && avHardwareContext)
		{
			HRESULT hr = DirectXInteropHelper::GetDeviceManagerFromStreamSource(sender, &deviceManager);
			if (SUCCEEDED(hr)) hr = D3D11VideoSampleProvider::InitializeHardwareDeviceContext(sender, avHardwareContext, &device, &deviceContext, deviceManager, &deviceHandle);

			if (SUCCEEDED(hr))
			{
				// assign device and context
				for each (auto stream in videoStreams)
				{
					// set device pointers to stream
					hr = stream->SetHardwareDevice(device, deviceContext, avHardwareContext);

					if (!SUCCEEDED(hr))
					{
						break;
					}
				}
			}
			else
			{
				// unref all hw device contexts
				for each (auto stream in videoStreams)
				{
					if (stream->m_pAvCodecCtx->hw_device_ctx)
					{
						av_buffer_unref(&stream->m_pAvCodecCtx->hw_device_ctx);
					}
				}
				av_buffer_unref(&avHardwareContext);
				SAFE_RELEASE(device);
				SAFE_RELEASE(deviceContext);
			}
		}

		// Perform seek operation when MediaStreamSource received seek event from MediaElement
		if (request.StartPosition() && request.StartPosition().Value().count() <= mediaDuration.count() && (!isFirstSeek || request.StartPosition().Value().count() > 0))
		{
			TimeSpan actualPosition = request.StartPosition().Value();
			auto hr = Seek(request.StartPosition().Value(), actualPosition, true);
			if (SUCCEEDED(hr))
			{
				request.SetActualStartPosition(actualPosition);
			}

			if (currentVideoStream && !currentVideoStream->IsEnabled())
			{
				currentVideoStream->EnableStream();
			}

			if (currentAudioStream && !currentAudioStream->IsEnabled())
			{
				currentAudioStream->EnableStream();
			}
		}

		isFirstSeek = false;
		isFirstSeekAfterStreamSwitch = false;
		mutexGuard.unlock();
	}

	void FFmpegMediaSource::OnSampleRequested(winrt::Windows::Media::Core::MediaStreamSource const& sender, winrt::Windows::Media::Core::MediaStreamSourceSampleRequestedEventArgs const& args)
	{
		mutexGuard.lock();
		if (mss != nullptr)
		{
			if (currentAudioStream && args.Request().StreamDescriptor() == currentAudioStream->StreamDescriptor())
			{
				auto sample = currentAudioStream->GetNextSample();
				args.Request().Sample(sample);
			}
			else if (currentVideoStream && args.Request().StreamDescriptor() == currentVideoStream->StreamDescriptor())
			{
				CheckVideoDeviceChanged();
				auto sample = currentVideoStream->GetNextSample();
				args.Request().Sample(sample);
			}
			else
			{
				args.Request().Sample(nullptr);
			}
		}
		mutexGuard.unlock();
	}


	void FFmpegMediaSource::CheckVideoDeviceChanged()
	{
		bool hasDeviceChanged = false;
		HRESULT hr = S_OK;
		if (currentVideoStream->device)
		{
			hr = deviceManager->TestDevice(deviceHandle);
			hasDeviceChanged = hr == MF_E_DXGI_NEW_VIDEO_DEVICE;
		}

		if (hasDeviceChanged && avHardwareContext)
		{
			hr = S_OK;
			av_buffer_unref(&avHardwareContext);
			SAFE_RELEASE(device);
			SAFE_RELEASE(deviceContext);

			if (deviceHandle && deviceManager)
				deviceManager->CloseDeviceHandle(deviceHandle);

			avHardwareContext = av_hwdevice_ctx_alloc(AVHWDeviceType::AV_HWDEVICE_TYPE_D3D11VA);

			if (!avHardwareContext)
			{
				hr = E_OUTOFMEMORY;
			}

			if (SUCCEEDED(hr))
			{
				hr = FFmpegInteropX::D3D11VideoSampleProvider::InitializeHardwareDeviceContext(mss, avHardwareContext, &device, &deviceContext, deviceManager, &deviceHandle);
			}

			if (SUCCEEDED(hr))
			{
				// assign device and context
				for each (auto stream in videoStreams)
				{
					// set device pointers to stream
					hr = stream->SetHardwareDevice(device, deviceContext, avHardwareContext);

					if (!SUCCEEDED(hr))
					{
						break;
					}
				}
			}

			if (SUCCEEDED(hr))
			{
				if (mss.CanSeek())
				{
					// seek to last keyframe position
					TimeSpan lastVideoTimestamp = currentVideoStream->LastSampleTimestamp;
					TimeSpan actualPosition;
					Seek(lastVideoTimestamp, actualPosition, false);

					// decode video until we are at target position
					while (true)
					{
						auto sample = currentVideoStream->GetNextSample();
						if (!sample || sample.Timestamp() >= lastVideoTimestamp)
						{
							break;
						}
					}

					// decode audio until we are at target position
					if (currentAudioStream)
					{
						TimeSpan lastAudioTimestamp = currentAudioStream->LastSampleTimestamp;
						while (true)
						{
							auto sample = currentAudioStream->GetNextSample();
							if (!sample || sample.Timestamp() >= lastAudioTimestamp)
							{
								break;
							}
						}
					}
				}
			}
		}
	}

	void FFmpegMediaSource::OnSwitchStreamsRequested(MediaStreamSource const& sender, MediaStreamSourceSwitchStreamsRequestedEventArgs const& args)
	{
		mutexGuard.lock();

		if (currentAudioStream && args.Request().OldStreamDescriptor() == currentAudioStream->StreamDescriptor())
		{
			if (!currentAudioEffects.empty())
			{
				currentAudioStream->DisableFilters();
			}
			currentAudioStream->DisableStream();
			currentAudioStream = nullptr;
		}
		if (currentVideoStream && args.Request().OldStreamDescriptor() == currentVideoStream->StreamDescriptor())
		{
			currentVideoStream->DisableStream();
			currentVideoStream = nullptr;
		}

		for each (auto stream in audioStreams)
		{
			if (stream->StreamDescriptor() == args.Request().NewStreamDescriptor())
			{
				currentAudioStream = stream;
				currentAudioStream->EnableStream();
				if (!currentAudioEffects.empty())
				{
					currentAudioStream->SetFilters(currentAudioEffects);
				}
			}
		}
		for each (auto stream in videoStreams)
		{
			if (stream->StreamDescriptor() == args.Request().NewStreamDescriptor())
			{
				currentVideoStream = stream;
				currentVideoStream->EnableStream();
			}
		}

		isFirstSeekAfterStreamSwitch = config.FastSeekSmartStreamSwitching();

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
