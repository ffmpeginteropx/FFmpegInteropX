//*****************************************************************************
//
//	Copyright 2015 Microsoft Corporation
//
//	Licensed under the Apache License, Version 2.0 (the "License");
//	you may not use this file except in compliance with the License.
//	You may obtain a copy of the License at
//
//	http ://www.apache.org/licenses/LICENSE-2.0
//
//	Unless required by applicable law or agreed to in writing, software
//	distributed under the License is distributed on an "AS IS" BASIS,
//	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//	See the License for the specific language governing permissions and
//	limitations under the License.
//
//*****************************************************************************

#include "pch.h"
#include "FFmpegInteropMSS.h"
#include "CompressedSampleProvider.h"
#include "H264AVCSampleProvider.h"
#include "NALPacketSampleProvider.h"
#include "HEVCSampleProvider.h"
#include "UncompressedAudioSampleProvider.h"
#include "UncompressedVideoSampleProvider.h"
#include "SubtitleProviderSsaAss.h"
#include "SubtitleProviderBitmap.h"
#include "CritSec.h"
#include "shcore.h"
#include <mfapi.h>
#include <dshow.h>
#include "LanguageTagConverter.h"
#include "FFmpegVersionInfo.h"
#include "collection.h"
#include <ppl.h>

extern "C"
{
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
}

using namespace concurrency;
using namespace FFmpegInterop;
using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Storage::Streams;
using namespace Windows::Media::MediaProperties;
using namespace Windows::Foundation::Collections;
using namespace Windows::ApplicationModel::Core;

// Static functions passed to FFmpeg
static int FileStreamRead(void* ptr, uint8_t* buf, int bufSize);
static int64_t FileStreamSeek(void* ptr, int64_t pos, int whence);

// Flag for ffmpeg global setup
static bool isRegistered = false;
std::mutex isRegisteredMutex;

std::map<String^, LanguageEntry^> LanguageTagConverter::map;

// Initialize an FFmpegInteropObject
FFmpegInteropMSS::FFmpegInteropMSS(FFmpegInteropConfig^ interopConfig, CoreDispatcher^ dispatcher)
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
			FFmpegVersionInfo::CheckMinimumVersion();
		}

		isRegisteredMutex.unlock();
	}
	subtitleDelay = config->DefaultSubtitleDelay;
	audioStrInfos = ref new Vector<AudioStreamInfo^>();
	subtitleStrInfos = ref new Vector<SubtitleStreamInfo^>();
	if (!config->IsExternalSubtitleParser && !config->IsFrameGrabber)
	{
		metadata = ref new MediaMetadata();
	}
}

FFmpegInteropMSS::~FFmpegInteropMSS()
{
	mutexGuard.lock();
	if (mss)
	{
		mss->Starting -= startingRequestedToken;
		mss->SampleRequested -= sampleRequestedToken;
		mss->SwitchStreamsRequested -= switchStreamRequestedToken;
		mss = nullptr;
	}

	if (playbackItem)
	{
		playbackItem->AudioTracksChanged -= audioTracksChangedToken;
		playbackItem->TimedMetadataTracks->PresentationModeChanged -= subtitlePresentationModeChangedToken;
		playbackItem = nullptr;
	}

	// Clear our data
	currentAudioStream = nullptr;
	videoStream = nullptr;

	if (m_pReader != nullptr)
	{
		m_pReader = nullptr;
	}

	sampleProviders.clear();
	audioStreams.clear();

	avformat_close_input(&avFormatCtx);
	av_free(avIOCtx);
	av_dict_free(&avDict);

	if (fileStreamData != nullptr)
	{
		fileStreamData->Release();
	}
	mutexGuard.unlock();
}

IAsyncOperation<FFmpegInteropMSS^>^ FFmpegInteropMSS::CreateFromStreamAsync(IRandomAccessStream^ stream, FFmpegInteropConfig^ config)
{
	auto dispatcher = GetCurrentDispatcher();
	return create_async([stream, config, dispatcher]
	{
		return CreateFromStream(stream, config, nullptr, dispatcher);
	});
};

IAsyncOperation<FFmpegInteropMSS^>^ FFmpegInteropMSS::CreateFromUriAsync(String^ uri, FFmpegInteropConfig^ config)
{
	auto dispatcher = GetCurrentDispatcher();
	return create_async([uri, config, dispatcher]
	{
		return CreateFromUri(uri, config, dispatcher);
	});
};

FFmpegInteropMSS^ FFmpegInteropMSS::CreateFromStream(IRandomAccessStream^ stream, FFmpegInteropConfig^ config, MediaStreamSource^ mss, CoreDispatcher^ dispatcher)
{
	auto interopMSS = ref new FFmpegInteropMSS(config, dispatcher);
	auto hr = interopMSS->CreateMediaStreamSource(stream, mss);
	if (!SUCCEEDED(hr))
	{
		throw ref new Exception(hr, "Failed to open media.");
	}
	return interopMSS;
}

FFmpegInteropMSS^ FFmpegInteropMSS::CreateFromUri(String^ uri, FFmpegInteropConfig^ config, CoreDispatcher^ dispatcher)
{
	auto interopMSS = ref new FFmpegInteropMSS(config, dispatcher);
	auto hr = interopMSS->CreateMediaStreamSource(uri);
	if (!SUCCEEDED(hr))
	{
		throw ref new Exception(hr, "Failed to open media.");
	}
	return interopMSS;
}

FFmpegInteropMSS^ FFmpegInteropMSS::CreateFFmpegInteropMSSFromStream(IRandomAccessStream^ stream, bool forceAudioDecode, bool forceVideoDecode, PropertySet^ ffmpegOptions, MediaStreamSource^ mss)
{
	auto config = ref new FFmpegInteropConfig();
	config->PassthroughAudioAAC = !forceAudioDecode;
	config->PassthroughAudioMP3 = !forceAudioDecode;
	config->PassthroughVideoH264 = !forceVideoDecode;
	config->PassthroughVideoH264Hi10P = !forceVideoDecode;
	config->PassthroughVideoHEVC = !forceVideoDecode;
	config->PassthroughVideoMPEG2 = !forceVideoDecode;
	config->PassthroughVideoVC1 = !forceVideoDecode;
	config->PassthroughVideoVP9 = !forceVideoDecode;
	config->PassthroughVideoWMV3 = !forceVideoDecode;
	if (ffmpegOptions != nullptr)
	{
		config->FFmpegOptions = ffmpegOptions;
	}
	try
	{
		auto dispatcher = GetCurrentDispatcher();
		return CreateFromStream(stream, config, nullptr, dispatcher);
	}
	catch (...)
	{
		return nullptr;
	}
}

FFmpegInteropMSS^ FFmpegInteropMSS::CreateFFmpegInteropMSSFromStream(IRandomAccessStream^ stream, bool forceAudioDecode, bool forceVideoDecode, PropertySet^ ffmpegOptions)
{
#pragma warning(suppress:4973)
	return CreateFFmpegInteropMSSFromStream(stream, forceAudioDecode, forceVideoDecode, nullptr, nullptr);
}

FFmpegInteropMSS^ FFmpegInteropMSS::CreateFFmpegInteropMSSFromStream(IRandomAccessStream^ stream, bool forceAudioDecode, bool forceVideoDecode)
{
#pragma warning(suppress:4973)
	return CreateFFmpegInteropMSSFromStream(stream, forceAudioDecode, forceVideoDecode, nullptr);
}

FFmpegInteropMSS^ FFmpegInteropMSS::CreateFFmpegInteropMSSFromUri(String^ uri, bool forceAudioDecode, bool forceVideoDecode, PropertySet^ ffmpegOptions)
{
	auto config = ref new FFmpegInteropConfig();
	config->PassthroughAudioAAC = !forceAudioDecode;
	config->PassthroughAudioMP3 = !forceAudioDecode;
	config->PassthroughVideoH264 = !forceVideoDecode;
	config->PassthroughVideoH264Hi10P = !forceVideoDecode;
	config->PassthroughVideoHEVC = !forceVideoDecode;
	config->PassthroughVideoMPEG2 = !forceVideoDecode;
	config->PassthroughVideoVC1 = !forceVideoDecode;
	config->PassthroughVideoVP9 = !forceVideoDecode;
	config->PassthroughVideoWMV3 = !forceVideoDecode;
	if (ffmpegOptions != nullptr)
	{
		config->FFmpegOptions = ffmpegOptions;
	}
	try
	{
		auto dispatcher = GetCurrentDispatcher();
		return CreateFromUri(uri, config, dispatcher);
	}
	catch (...)
	{
		return nullptr;
	}
}

FFmpegInteropMSS^ FFmpegInteropMSS::CreateFFmpegInteropMSSFromUri(String^ uri, bool forceAudioDecode, bool forceVideoDecode)
{
#pragma warning(suppress:4973)
	return CreateFFmpegInteropMSSFromUri(uri, forceAudioDecode, forceVideoDecode, nullptr);
}

MediaStreamSource^ FFmpegInteropMSS::GetMediaStreamSource()
{
	if (this->config->IsFrameGrabber) throw ref new Exception(E_UNEXPECTED);
	return mss;
}

MediaSource^ FFmpegInteropMSS::CreateMediaSource()
{
	if (this->config->IsFrameGrabber) throw ref new Exception(E_UNEXPECTED);
	MediaSource^ source = MediaSource::CreateFromMediaStreamSource(mss);
	for each (auto stream in subtitleStreams)
	{
		source->ExternalTimedMetadataTracks->Append(stream->SubtitleTrack);
	}
	for each(auto subtitleInfo in SubtitleStreams)
	{
		if (subtitleInfo->IsExternal) {
			source->ExternalTimedMetadataTracks->Append(subtitleInfo->SubtitleTrack);
		}
	}
	return source;
}

MediaPlaybackItem^ FFmpegInteropMSS::CreateMediaPlaybackItem()
{
	mutexGuard.lock();
	try
	{
		if (this->config->IsFrameGrabber || playbackItem != nullptr) throw ref new Exception(E_UNEXPECTED);
		playbackItem = ref new MediaPlaybackItem(CreateMediaSource());
		InitializePlaybackItem(playbackItem);
		mutexGuard.unlock();
		return playbackItem;
	}
	catch (...)
	{
		mutexGuard.unlock();
		throw;
	}
}

MediaPlaybackItem^ FFmpegInteropMSS::CreateMediaPlaybackItem(TimeSpan startTime)
{
	mutexGuard.lock();
	try
	{
		if (this->config->IsFrameGrabber || playbackItem != nullptr) throw ref new Exception(E_UNEXPECTED);
		playbackItem = ref new MediaPlaybackItem(CreateMediaSource(), startTime);
		InitializePlaybackItem(playbackItem);
		mutexGuard.unlock();
		return playbackItem;
	}
	catch (...)
	{
		mutexGuard.unlock();
		throw;
	}
}

MediaPlaybackItem^ FFmpegInteropMSS::CreateMediaPlaybackItem(TimeSpan startTime, TimeSpan durationLimit)
{
	mutexGuard.lock();
	try
	{
		if (this->config->IsFrameGrabber || playbackItem != nullptr) throw ref new Exception(E_UNEXPECTED);
		playbackItem = ref new MediaPlaybackItem(CreateMediaSource(), startTime, durationLimit);
		InitializePlaybackItem(playbackItem);
		mutexGuard.unlock();
		return playbackItem;
	}
	catch (...)
	{
		mutexGuard.unlock();
		throw;
	}
}

IAsyncOperation<IVectorView<SubtitleStreamInfo^>^>^ FFmpegInteropMSS::AddExternalSubtitleAsync(IRandomAccessStream ^ stream, String^ streamName)
{
	return create_async([this, stream, streamName]
	{
		auto subConfig = ref new FFmpegInteropConfig();
		subConfig->IsExternalSubtitleParser = true;
		subConfig->DefaultSubtitleStreamName = streamName;
		subConfig->DefaultSubtitleDelay = this->SubtitleDelay;
		subConfig->AutoCorrectAnsiSubtitles = this->config->AutoCorrectAnsiSubtitles;
		subConfig->AnsiSubtitleEncoding = this->config->AnsiSubtitleEncoding;
		subConfig->OverrideSubtitleStyles = this->config->OverrideSubtitleStyles;
		subConfig->SubtitleRegion = this->config->SubtitleRegion;
		subConfig->SubtitleStyle = this->config->SubtitleStyle;
		subConfig->AutoCorrectAnsiSubtitles = this->config->AutoCorrectAnsiSubtitles;
		subConfig->AutoSelectForcedSubtitles = false;

		if (VideoDescriptor)
		{
			subConfig->AdditionalFFmpegSubtitleOptions = ref new PropertySet();

			subConfig->AdditionalFFmpegSubtitleOptions->Insert("subfps",
				VideoDescriptor->EncodingProperties->FrameRate->Numerator.ToString() + "/" + VideoDescriptor->EncodingProperties->FrameRate->Denominator.ToString());
		}
		auto externalSubsParser = FFmpegInteropMSS::CreateFromStream(stream, subConfig, nullptr, nullptr);

		if (externalSubsParser->SubtitleStreams->Size > 0)
		{
			if (VideoDescriptor)
			{
				auto pixelAspect = (double)VideoDescriptor->EncodingProperties->PixelAspectRatio->Numerator / VideoDescriptor->EncodingProperties->PixelAspectRatio->Denominator;
				auto videoAspect = ((double)videoStream->m_pAvCodecCtx->width / videoStream->m_pAvCodecCtx->height) / pixelAspect;
				for each (auto stream in externalSubsParser->subtitleStreams)
				{
					stream->NotifyVideoFrameSize(videoStream->m_pAvCodecCtx->width, videoStream->m_pAvCodecCtx->height, videoAspect);
				}
			}

			int readResult = 0;
			while ((readResult = externalSubsParser->m_pReader->ReadPacket()) >= 0)
			{
				Concurrency::interruption_point();
			}
		}


		mutexGuard.lock();
		try
		{
			if (SubtitleDelay.Duration != externalSubsParser->SubtitleDelay.Duration)
			{
				externalSubsParser->SetSubtitleDelay(SubtitleDelay);
			}

			int subtitleTracksCount = 0;

			for each(auto externalSubtitle in externalSubsParser->subtitleStreams)
			{
				if (externalSubtitle->SubtitleTrack->Cues->Size > 0)
				{
					// detach stream
					externalSubtitle->Detach();

					// find and add stream info
					for each (auto subtitleInfo in externalSubsParser->SubtitleStreams)
					{
						if (subtitleInfo->SubtitleTrack == externalSubtitle->SubtitleTrack)
						{
							subtitleStrInfos->Append(subtitleInfo);
							break;
						}
					}

					// add stream
					subtitleStreams.push_back(externalSubtitle);
					if (this->PlaybackItem != nullptr)
					{
						PlaybackItem->Source->ExternalTimedMetadataTracks->Append(externalSubtitle->SubtitleTrack);
					}
					subtitleTracksCount++;
				}
			}

			if (subtitleTracksCount == 0)
			{
				throw ref new InvalidArgumentException("No subtitles found in file.");
			}

			subtitleStreamInfos = subtitleStrInfos->GetView();
		}
		catch (...)
		{
			mutexGuard.unlock();
			throw;
		}
		mutexGuard.unlock();
		return externalSubsParser->SubtitleStreams;
	});
}

void FFmpegInteropMSS::InitializePlaybackItem(MediaPlaybackItem^ playbackitem)
{
	audioTracksChangedToken = playbackitem->AudioTracksChanged += ref new Windows::Foundation::TypedEventHandler<Windows::Media::Playback::MediaPlaybackItem ^, Windows::Foundation::Collections::IVectorChangedEventArgs ^>(this, &FFmpegInterop::FFmpegInteropMSS::OnAudioTracksChanged);
	subtitlePresentationModeChangedToken = playbackitem->TimedMetadataTracks->PresentationModeChanged += ref new Windows::Foundation::TypedEventHandler<Windows::Media::Playback::MediaPlaybackTimedMetadataTrackList ^, Windows::Media::Playback::TimedMetadataPresentationModeChangedEventArgs ^>(this, &FFmpegInterop::FFmpegInteropMSS::OnPresentationModeChanged);

	if (config->AutoSelectForcedSubtitles)
	{
		int index = 0;
		for each (auto stream in subtitleStreams)
		{
			if (subtitleStreamInfos->GetAt(index)->IsForced)
			{
				playbackitem->TimedMetadataTracks->SetPresentationMode(index, TimedMetadataTrackPresentationMode::PlatformPresented);
				break;
			}

			index++;
		}
	}

	for each (auto stream in subtitleStreams)
	{
		stream->PlaybackItem = playbackItem;
	}
}

void FFmpegInteropMSS::OnPresentationModeChanged(MediaPlaybackTimedMetadataTrackList ^sender, TimedMetadataPresentationModeChangedEventArgs ^args)
{
	mutexGuard.lock();
	int index = 0;
	for each (auto stream in subtitleStreams)
	{
		if (stream->SubtitleTrack == args->Track)
		{
			auto mode = args->NewPresentationMode;
			if (mode == TimedMetadataTrackPresentationMode::Disabled || mode == TimedMetadataTrackPresentationMode::Hidden)
			{
				stream->DisableStream();
			}
			else
			{
				stream->EnableStream();
			}
		}
		index++;
	}
	mutexGuard.unlock();
}

void FFmpegInteropMSS::OnAudioTracksChanged(MediaPlaybackItem ^sender, IVectorChangedEventArgs ^args)
{
	mutexGuard.lock();
	if (sender->AudioTracks->Size == AudioStreams->Size)
	{
		for (unsigned int i = 0; i < AudioStreams->Size; i++)
		{
			auto track = sender->AudioTracks->GetAt(i);
			auto info = AudioStreams->GetAt(i);
			if (info->Name != nullptr)
			{
				track->Label = info->Name;
			}
			else if (info->Language != nullptr)
			{
				track->Label = info->Language;
			}
		}
	}
	mutexGuard.unlock();
}

HRESULT FFmpegInteropMSS::CreateMediaStreamSource(String^ uri)
{
	HRESULT hr = S_OK;
	const char* charStr = nullptr;
	if (!uri)
	{
		hr = E_INVALIDARG;
	}

	if (SUCCEEDED(hr))
	{
		avFormatCtx = avformat_alloc_context();
		if (avFormatCtx == nullptr)
		{
			hr = E_OUTOFMEMORY;
		}
	}

	if (SUCCEEDED(hr))
	{
		// Populate AVDictionary avDict based on PropertySet ffmpegOptions. List of options can be found in https://www.ffmpeg.org/ffmpeg-protocols.html
		hr = ParseOptions(config->FFmpegOptions);
	}

	if (SUCCEEDED(hr))
	{
		std::wstring uriW(uri->Begin());
		std::string uriA(uriW.begin(), uriW.end());
		charStr = uriA.c_str();

		// Open media in the given URI using the specified options
		if (avformat_open_input(&avFormatCtx, charStr, NULL, &avDict) < 0)
		{
			hr = E_FAIL; // Error opening file
		}

		// avDict is not NULL only when there is an issue with the given ffmpegOptions such as invalid key, value type etc. Iterate through it to see which one is causing the issue.
		if (avDict != nullptr)
		{
			DebugMessage(L"Invalid FFmpeg option(s)");
			av_dict_free(&avDict);
			avDict = nullptr;
		}
	}

	if (SUCCEEDED(hr))
	{
		this->mss = nullptr;
		hr = InitFFmpegContext();
	}

	return hr;
}

HRESULT FFmpegInteropMSS::CreateMediaStreamSource(IRandomAccessStream^ stream, MediaStreamSource^ mss)
{
	HRESULT hr = S_OK;
	if (!stream)
	{
		hr = E_INVALIDARG;
	}

	if (SUCCEEDED(hr))
	{
		// Convert asynchronous IRandomAccessStream to synchronous IStream. This API requires shcore.h and shcore.lib
		hr = CreateStreamOverRandomAccessStream(reinterpret_cast<IUnknown*>(stream), IID_PPV_ARGS(&fileStreamData));
	}

	if (SUCCEEDED(hr))
	{
		// Setup FFmpeg custom IO to access file as stream. This is necessary when accessing any file outside of app installation directory and appdata folder.
		// Credit to Philipp Sch http://www.codeproject.com/Tips/489450/Creating-Custom-FFmpeg-IO-Context
		fileStreamBuffer = (unsigned char*)av_malloc(config->StreamBufferSize);
		if (fileStreamBuffer == nullptr)
		{
			hr = E_OUTOFMEMORY;
		}
	}

	if (SUCCEEDED(hr))
	{
		avIOCtx = avio_alloc_context(fileStreamBuffer, config->StreamBufferSize, 0, (void*)this, FileStreamRead, 0, FileStreamSeek);
		if (avIOCtx == nullptr)
		{
			hr = E_OUTOFMEMORY;
		}
	}

	if (SUCCEEDED(hr))
	{
		avFormatCtx = avformat_alloc_context();
		if (avFormatCtx == nullptr)
		{
			hr = E_OUTOFMEMORY;
		}
	}

	if (SUCCEEDED(hr))
	{
		// Populate AVDictionary avDict based on PropertySet ffmpegOptions. List of options can be found in https://www.ffmpeg.org/ffmpeg-protocols.html
		hr = ParseOptions(config->FFmpegOptions);
	}


	if (SUCCEEDED(hr))
	{
		// Populate AVDictionary avDict based on additional ffmpegOptions. List of options can be found in https://www.ffmpeg.org/ffmpeg-protocols.html
		hr = ParseOptions(config->AdditionalFFmpegSubtitleOptions);
	}

	if (SUCCEEDED(hr))
	{
		avFormatCtx->pb = avIOCtx;
		avFormatCtx->flags |= AVFMT_FLAG_CUSTOM_IO;

		// Open media file using custom IO setup above instead of using file name. Opening a file using file name will invoke fopen C API call that only have
		// access within the app installation directory and appdata folder. Custom IO allows access to file selected using FilePicker dialog.
		if (avformat_open_input(&avFormatCtx, "", NULL, &avDict) < 0)
		{
			hr = E_FAIL; // Error opening file
		}

		// avDict is not NULL only when there is an issue with the given ffmpegOptions such as invalid key, value type etc. Iterate through it to see which one is causing the issue.
		if (avDict != nullptr)
		{
			DebugMessage(L"Invalid FFmpeg option(s)");
			av_dict_free(&avDict);

			avDict = nullptr;
		}
	}

	if (SUCCEEDED(hr))
	{
		this->mss = mss;
		hr = InitFFmpegContext();
	}

	return hr;
}

static int is_hwaccel_pix_fmt(enum AVPixelFormat pix_fmt)
{
	const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(pix_fmt);
	return desc->flags & AV_PIX_FMT_FLAG_HWACCEL;
}

static AVPixelFormat get_format(struct AVCodecContext *s, const enum AVPixelFormat *fmt)
{
	AVPixelFormat result = (AVPixelFormat)-1;
	AVPixelFormat format;
	int index = 0;
	do
	{
		format = fmt[index++];
		if (format != -1 && result == -1 && !is_hwaccel_pix_fmt(format))
		{
			// take first non hw accelerated format
			result = format;
		}
		else if (format == AV_PIX_FMT_NV12 && result != AV_PIX_FMT_YUVA420P)
		{
			// switch to NV12 if available, unless this is an alpha channel file
			result = format;
		}
	} while (format != -1);
	return result;
}

HRESULT FFmpegInteropMSS::InitFFmpegContext()
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
		m_pReader = ref new FFmpegReader(avFormatCtx, &sampleProviders);
		if (m_pReader == nullptr)
		{
			hr = E_OUTOFMEMORY;
		}
	}

	// do not use start time for pure subtitle files
	if (config->IsExternalSubtitleParser && avFormatCtx->nb_streams == 1)
	{
		avFormatCtx->start_time = AV_NOPTS_VALUE;
	}

	AVCodec* avVideoCodec;
	auto videoStreamIndex = av_find_best_stream(avFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &avVideoCodec, 0);
	auto audioStreamIndex = av_find_best_stream(avFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
	auto subtitleStreamIndex = av_find_best_stream(avFormatCtx, AVMEDIA_TYPE_SUBTITLE, -1, -1, NULL, 0);

	attachedFileHelper = ref new AttachedFileHelper(config);

	// first parse attached files, so they are available for subtitle streams during initialize
	if (config->UseEmbeddedSubtitleFonts)
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
					auto name = ConvertString(fileName->value);
					auto mime = mimetype ? ConvertString(mimetype->value) : "";

					auto file = ref new AttachedFile(name, mime, avStream);
					attachedFileHelper->AddAttachedFile(file);
				}
			}
		}
	}

	for (unsigned int index = 0; index < avFormatCtx->nb_streams; index++)
	{
		auto avStream = avFormatCtx->streams[index];
		MediaSampleProvider^ stream;

		if (avStream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO && !config->IsFrameGrabber && !config->IsExternalSubtitleParser)
		{
			stream = CreateAudioStream(avStream, index);
			if (stream)
			{
				bool isDefault = index == audioStreamIndex;

				// TODO get info from sample provider
				auto channels = avStream->codecpar->channels;
				if (channels == 1 && avStream->codecpar->codec_id == AV_CODEC_ID_AAC && avStream->codecpar->profile == FF_PROFILE_AAC_HE_V2)
				{
					channels = 2;
				}
				auto info = ref new AudioStreamInfo(stream->Name, stream->Language, stream->CodecName, avStream->codecpar->bit_rate, isDefault,
					channels, avStream->codecpar->sample_rate,
					max(avStream->codecpar->bits_per_raw_sample, avStream->codecpar->bits_per_coded_sample));
				if (isDefault)
				{
					currentAudioStream = stream;
					audioStrInfos->InsertAt(0, info);
					audioStreams.insert(audioStreams.begin(), stream);
				}
				else
				{
					audioStrInfos->Append(info);
					audioStreams.push_back(stream);
				}
			}
		}
		else if (avStream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO && avStream->disposition == AV_DISPOSITION_ATTACHED_PIC && thumbnailStreamIndex == AVERROR_STREAM_NOT_FOUND && !config->IsExternalSubtitleParser)
		{
			thumbnailStreamIndex = index;
		}
		else if (avStream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO && index == videoStreamIndex && !config->IsExternalSubtitleParser)
		{
			videoStream = stream = CreateVideoStream(avStream, index);

			if (videoStream)
			{
				videoStreamInfo = ref new VideoStreamInfo(stream->Name, stream->Language, stream->CodecName, avStream->codecpar->bit_rate, true,
					avStream->codecpar->width, avStream->codecpar->height,
					max(avStream->codecpar->bits_per_raw_sample, avStream->codecpar->bits_per_coded_sample));
			}
		}
		else if (avStream->codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE)
		{
			stream = CreateSubtitleSampleProvider(avStream, index);
			if (stream)
			{
				auto isDefault = index == subtitleStreamIndex;
				auto info = ref new SubtitleStreamInfo(stream->Name, stream->Language, stream->CodecName,
					isDefault, (avStream->disposition & AV_DISPOSITION_FORCED) == AV_DISPOSITION_FORCED, ((SubtitleProvider^)stream)->SubtitleTrack, config->IsExternalSubtitleParser);
				if (isDefault)
				{
					subtitleStrInfos->InsertAt(0, info);
					subtitleStreams.insert(subtitleStreams.begin(), (SubtitleProvider^)stream);
				}
				else
				{
					subtitleStrInfos->Append(info);
					subtitleStreams.push_back((SubtitleProvider^)stream);
				}

				// enable all subtitle streams for external subtitle parsing
				if (config->IsExternalSubtitleParser)
				{
					((SubtitleProvider^)stream)->EnableStream();
				}
			}
		}

		sampleProviders.push_back(stream);
	}

	if (!currentAudioStream && audioStreams.size() > 0)
	{
		currentAudioStream = audioStreams[0];
	}

	audioStreamInfos = audioStrInfos->GetView();
	subtitleStreamInfos = subtitleStrInfos->GetView();

	if (videoStream)
	{
		auto pixelAspect = (double)VideoDescriptor->EncodingProperties->PixelAspectRatio->Numerator / VideoDescriptor->EncodingProperties->PixelAspectRatio->Denominator;
		auto videoAspect = ((double)videoStream->m_pAvCodecCtx->width / videoStream->m_pAvCodecCtx->height) / pixelAspect;
		for each (auto stream in subtitleStreams)
		{
			stream->NotifyVideoFrameSize(videoStream->m_pAvCodecCtx->width, videoStream->m_pAvCodecCtx->height, videoAspect);
		}
	}

	if (videoStream && currentAudioStream)
	{
		mss = ref new MediaStreamSource(videoStream->StreamDescriptor, currentAudioStream->StreamDescriptor);
		videoStream->EnableStream();
		currentAudioStream->EnableStream();
	}
	else if (currentAudioStream)
	{
		mss = ref new MediaStreamSource(currentAudioStream->StreamDescriptor);
		currentAudioStream->EnableStream();
	}
	else if (videoStream)
	{
		mss = ref new MediaStreamSource(videoStream->StreamDescriptor);
		videoStream->EnableStream();
	}
	else if (subtitleStreams.size() == 0 || !config->IsExternalSubtitleParser)
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
					mss->AddStreamDescriptor(stream->StreamDescriptor);
				}
			}
		}

		if (SUCCEEDED(hr))
		{
			// Convert media duration from AV_TIME_BASE to TimeSpan unit
			mediaDuration = { LONGLONG(avFormatCtx->duration * 10000000 / double(AV_TIME_BASE)) };

			TimeSpan buffer = { 0 };
			mss->BufferTime = buffer;

			if (Windows::Foundation::Metadata::ApiInformation::IsPropertyPresent("Windows.Media.Core.MediaStreamSource", "MaxSupportedPlaybackRate"))
			{
				mss->MaxSupportedPlaybackRate = config->MaxSupportedPlaybackRate;
			}

			if (mediaDuration.Duration > 0)
			{
				mss->Duration = mediaDuration;
				mss->CanSeek = true;
			}

			startingRequestedToken = mss->Starting += ref new TypedEventHandler<MediaStreamSource ^, MediaStreamSourceStartingEventArgs ^>(this, &FFmpegInteropMSS::OnStarting);
			sampleRequestedToken = mss->SampleRequested += ref new TypedEventHandler<MediaStreamSource ^, MediaStreamSourceSampleRequestedEventArgs ^>(this, &FFmpegInteropMSS::OnSampleRequested);
			switchStreamRequestedToken = mss->SwitchStreamsRequested += ref new TypedEventHandler<MediaStreamSource ^, MediaStreamSourceSwitchStreamsRequestedEventArgs ^>(this, &FFmpegInteropMSS::OnSwitchStreamsRequested);
		}
	}
	
	return hr;
}

SubtitleProvider^ FFmpegInteropMSS::CreateSubtitleSampleProvider(AVStream * avStream, int index)
{
	HRESULT hr = S_OK;
	SubtitleProvider^ avSubsStream = nullptr;
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
		if (config->AutoCorrectAnsiSubtitles && config->IsExternalSubtitleParser && streamByteOrderMark != ByteOrderMark::UTF8)
		{
			String^ key = config->AnsiSubtitleEncoding->Name;
			std::wstring keyW(key->Begin());
			std::string keyA(keyW.begin(), keyW.end());
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
						avSubsStream = ref new SubtitleProviderSsaAss(m_pReader, avFormatCtx, avSubsCodecCtx, config, index, dispatcher, attachedFileHelper);
					}
					else if ((avSubsCodecCtx->codec_descriptor->props & AV_CODEC_PROP_BITMAP_SUB) == AV_CODEC_PROP_BITMAP_SUB)
					{
						if (Windows::Foundation::Metadata::ApiInformation::IsEnumNamedValuePresent("Windows.Media.Core.TimedMetadataKind", "ImageSubtitle"))
						{
							avSubsStream = ref new SubtitleProviderBitmap(m_pReader, avFormatCtx, avSubsCodecCtx, config, index, dispatcher);
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
			avSubsStream->SubtitleDelay = SubtitleDelay;
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

MediaSampleProvider^ FFmpegInteropMSS::CreateAudioStream(AVStream * avStream, int index)
{
	HRESULT hr = S_OK;
	MediaSampleProvider^ audioStream = nullptr;
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
					avAudioCodecCtx->thread_count = config->MaxAudioThreads == 0 ? threads : min(threads, config->MaxAudioThreads);
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

MediaSampleProvider^ FFmpegInteropMSS::CreateVideoStream(AVStream * avStream, int index)
{
	HRESULT hr = S_OK;
	MediaSampleProvider^ result = nullptr;

	// Find the video stream and its decoder
	auto avVideoCodec = avcodec_find_decoder(avStream->codecpar->codec_id);

	if (avVideoCodec)
	{
		// allocate a new decoding context
		auto avVideoCodecCtx = avcodec_alloc_context3(avVideoCodec);
		if (!avVideoCodecCtx)
		{
			DebugMessage(L"Could not allocate a decoding context\n");
			hr = E_OUTOFMEMORY;
		}

		if (SUCCEEDED(hr))
		{
			avVideoCodecCtx->get_format = &get_format;

			// initialize the stream parameters with demuxer information
			if (avcodec_parameters_to_context(avVideoCodecCtx, avStream->codecpar) < 0)
			{
				hr = E_FAIL;
			}
		}

		if (SUCCEEDED(hr))
		{
			// enable multi threading
			unsigned threads = std::thread::hardware_concurrency();
			if (threads > 0)
			{
				avVideoCodecCtx->thread_count = config->MaxVideoThreads == 0 ? threads : min(threads, config->MaxVideoThreads);
				avVideoCodecCtx->thread_type = config->IsFrameGrabber ? FF_THREAD_SLICE : FF_THREAD_FRAME | FF_THREAD_SLICE;
			}

			if (avcodec_open2(avVideoCodecCtx, avVideoCodec, NULL) < 0)
			{
				hr = E_FAIL;
			}
			else if (avVideoCodecCtx->pix_fmt == AV_PIX_FMT_NONE)
			{
				hr = E_FAIL;
			}
			else
			{
				// Detect video format and create video stream descriptor accordingly
				result = CreateVideoSampleProvider(avStream, avVideoCodecCtx, index);
			}
		}

		// free codec context if failed
		if (!result && avVideoCodecCtx)
		{
			avcodec_free_context(&avVideoCodecCtx);
		}
	}

	return result;
}

void FFmpegInteropMSS::SetSubtitleDelay(TimeSpan offset)
{
	mutexGuard.lock();
	try
	{
		for each(auto subtitleStream in subtitleStreams)
		{
			subtitleStream->SetSubtitleDelay(offset);
		}

		subtitleDelay = offset;
	}
	catch (...)
	{
	}
	mutexGuard.unlock();
}

void FFmpegInteropMSS::SetAudioEffects(IVectorView<AvEffectDefinition^>^ audioEffects)
{
	mutexGuard.lock();
	if (currentAudioStream)
	{
		currentAudioStream->SetFilters(audioEffects);
		currentAudioEffects = audioEffects;
	}
	mutexGuard.unlock();
}

void FFmpegInteropMSS::SetVideoEffects(IVectorView<AvEffectDefinition^>^ videoEffects)
{
	mutexGuard.lock();
	if (videoStream)
	{
		videoStream->SetFilters(videoEffects);
	}
	mutexGuard.unlock();
}

void FFmpegInteropMSS::DisableAudioEffects()
{
	mutexGuard.lock();
	if (currentAudioStream)
	{
		currentAudioStream->DisableFilters();
		currentAudioEffects = nullptr;
	}
	mutexGuard.unlock();
}

void FFmpegInteropMSS::DisableVideoEffects()
{
	mutexGuard.lock();
	if (videoStream)
	{
		videoStream->DisableFilters();
	}
	mutexGuard.unlock();
}

MediaThumbnailData ^ FFmpegInteropMSS::ExtractThumbnail()
{
	if (thumbnailStreamIndex != AVERROR_STREAM_NOT_FOUND)
	{
		// FFmpeg identifies album/cover art from a music file as a video stream
		// Avoid creating unnecessarily video stream from this album/cover art
		if (avFormatCtx->streams[thumbnailStreamIndex]->disposition == AV_DISPOSITION_ATTACHED_PIC)
		{
			auto imageStream = avFormatCtx->streams[thumbnailStreamIndex];
			//save album art to file.
			String^ extension = ".jpeg";
			switch (imageStream->codecpar->codec_id)
			{
			case AV_CODEC_ID_MJPEG:
			case AV_CODEC_ID_MJPEGB:
			case AV_CODEC_ID_JPEG2000:
			case AV_CODEC_ID_JPEGLS: extension = ".jpeg"; break;
			case AV_CODEC_ID_PNG: extension = ".png"; break;
			case AV_CODEC_ID_BMP: extension = ".bmp"; break;
			}


			auto vector = ArrayReference<uint8_t>(imageStream->attached_pic.data, imageStream->attached_pic.size);
			DataWriter^ writer = ref new DataWriter();
			writer->WriteBytes(vector);

			return (ref new MediaThumbnailData(writer->DetachBuffer(), extension));
		}
	}

	return nullptr;
}

MediaSampleProvider^ FFmpegInteropMSS::CreateAudioSampleProvider(AVStream* avStream, AVCodecContext* avAudioCodecCtx, int index)
{
	MediaSampleProvider^ audioSampleProvider;
	if (avAudioCodecCtx->codec_id == AV_CODEC_ID_AAC && config->PassthroughAudioAAC)
	{
		AudioEncodingProperties^ encodingProperties;
		if (avAudioCodecCtx->extradata_size == 0)
		{
			encodingProperties = AudioEncodingProperties::CreateAacAdts(avAudioCodecCtx->sample_rate, avAudioCodecCtx->channels, (unsigned int)avAudioCodecCtx->bit_rate);
		}
		else
		{
			encodingProperties = AudioEncodingProperties::CreateAac(avAudioCodecCtx->profile == FF_PROFILE_AAC_HE || avAudioCodecCtx->profile == FF_PROFILE_AAC_HE_V2 ? avAudioCodecCtx->sample_rate / 2 : avAudioCodecCtx->sample_rate, avAudioCodecCtx->channels, (unsigned int)avAudioCodecCtx->bit_rate);
		}
		audioSampleProvider = ref new CompressedSampleProvider(m_pReader, avFormatCtx, avAudioCodecCtx, config, index, encodingProperties);
	}
	else if (avAudioCodecCtx->codec_id == AV_CODEC_ID_MP3 && config->PassthroughAudioMP3)
	{
		AudioEncodingProperties^ encodingProperties = AudioEncodingProperties::CreateMp3(avAudioCodecCtx->sample_rate, avAudioCodecCtx->channels, (unsigned int)avAudioCodecCtx->bit_rate);
		audioSampleProvider = ref new CompressedSampleProvider(m_pReader, avFormatCtx, avAudioCodecCtx, config, index, encodingProperties);
	}
	else
	{
		audioSampleProvider = ref new UncompressedAudioSampleProvider(m_pReader, avFormatCtx, avAudioCodecCtx, config, index);
	}

	auto hr = audioSampleProvider->Initialize();
	if (FAILED(hr))
	{
		audioSampleProvider = nullptr;
	}

	return audioSampleProvider;
}

MediaSampleProvider^ FFmpegInteropMSS::CreateVideoSampleProvider(AVStream* avStream, AVCodecContext* avVideoCodecCtx, int index)
{
	MediaSampleProvider^ videoSampleProvider;
	VideoEncodingProperties^ videoProperties;

	if (avVideoCodecCtx->codec_id == AV_CODEC_ID_H264 && config->PassthroughVideoH264 && !config->IsFrameGrabber && (avVideoCodecCtx->profile <= 100 || config->PassthroughVideoH264Hi10P))
	{
		auto videoProperties = VideoEncodingProperties::CreateH264();

		// Check for H264 bitstream flavor. H.264 AVC extradata starts with 1 while non AVC one starts with 0
		if (avVideoCodecCtx->extradata != nullptr && avVideoCodecCtx->extradata_size > 0 && avVideoCodecCtx->extradata[0] == 1)
		{
			videoSampleProvider = ref new H264AVCSampleProvider(m_pReader, avFormatCtx, avVideoCodecCtx, config, index, videoProperties);
		}
		else
		{
			videoSampleProvider = ref new NALPacketSampleProvider(m_pReader, avFormatCtx, avVideoCodecCtx, config, index, videoProperties);
		}
	}
	else if (avVideoCodecCtx->codec_id == AV_CODEC_ID_HEVC && config->PassthroughVideoHEVC && !config->IsFrameGrabber &&
		Windows::Foundation::Metadata::ApiInformation::IsMethodPresent("Windows.Media.MediaProperties.VideoEncodingProperties", "CreateHevc"))
	{
		auto videoProperties = VideoEncodingProperties::CreateHevc();

		// Check for HEVC bitstream flavor.
		if (avVideoCodecCtx->extradata != nullptr && avVideoCodecCtx->extradata_size > 22 &&
			(avVideoCodecCtx->extradata[0] || avVideoCodecCtx->extradata[1] || avVideoCodecCtx->extradata[2] > 1))
		{
			videoSampleProvider = ref new HEVCSampleProvider(m_pReader, avFormatCtx, avVideoCodecCtx, config, index, videoProperties);
		}
		else
		{
			videoSampleProvider = ref new NALPacketSampleProvider(m_pReader, avFormatCtx, avVideoCodecCtx, config, index, videoProperties);
		}
	}
	else if (avVideoCodecCtx->codec_id == AV_CODEC_ID_WMV3 && config->PassthroughVideoWMV3 && !config->IsFrameGrabber && avVideoCodecCtx->extradata_size > 0)
	{
		auto videoProperties = ref new VideoEncodingProperties();
		videoProperties->Subtype = MediaEncodingSubtypes::Wmv3;

		auto extradata = Platform::ArrayReference<uint8_t>(avVideoCodecCtx->extradata, avVideoCodecCtx->extradata_size);
		videoProperties->SetFormatUserData(extradata);
		videoSampleProvider = ref new CompressedSampleProvider(m_pReader, avFormatCtx, avVideoCodecCtx, config, index, videoProperties);
	}
	else if (avVideoCodecCtx->codec_id == AV_CODEC_ID_VC1 && config->PassthroughVideoVC1 && !config->IsFrameGrabber && avVideoCodecCtx->extradata_size > 0)
	{
		auto videoProperties = ref new VideoEncodingProperties();
		videoProperties->Subtype = MediaEncodingSubtypes::Wvc1;

		auto extradata = Platform::ArrayReference<uint8_t>(avVideoCodecCtx->extradata, avVideoCodecCtx->extradata_size);
		videoProperties->SetFormatUserData(extradata);
		videoSampleProvider = ref new CompressedSampleProvider(m_pReader, avFormatCtx, avVideoCodecCtx, config, index, videoProperties);
	}
	else if (avVideoCodecCtx->codec_id == AV_CODEC_ID_MPEG2VIDEO && config->PassthroughVideoMPEG2 && !config->IsFrameGrabber)
	{
		auto videoProperties = ref new VideoEncodingProperties();
		videoProperties->Subtype = MediaEncodingSubtypes::Mpeg2;

		videoSampleProvider = ref new CompressedSampleProvider(m_pReader, avFormatCtx, avVideoCodecCtx, config, index, videoProperties);
	}
	else if (avVideoCodecCtx->codec_id == AV_CODEC_ID_VP9 && config->PassthroughVideoVP9 && !config->IsFrameGrabber &&
		Windows::Foundation::Metadata::ApiInformation::IsPropertyPresent("Windows.Media.MediaProperties.MediaEncodingSubtypes", "Vp9"))
	{
		auto videoProperties = ref new VideoEncodingProperties();
		videoProperties->Subtype = MediaEncodingSubtypes::Vp9;

		videoSampleProvider = ref new CompressedSampleProvider(m_pReader, avFormatCtx, avVideoCodecCtx, config, index, videoProperties);
	}
	else
	{
		videoSampleProvider = ref new UncompressedVideoSampleProvider(m_pReader, avFormatCtx, avVideoCodecCtx, config, index);
	}

	auto hr = videoSampleProvider->Initialize();

	if (FAILED(hr))
	{
		videoSampleProvider = nullptr;
	}

	return videoSampleProvider;
}

HRESULT FFmpegInteropMSS::ParseOptions(PropertySet^ ffmpegOptions)
{
	HRESULT hr = S_OK;

	// Convert FFmpeg options given in PropertySet to AVDictionary. List of options can be found in https://www.ffmpeg.org/ffmpeg-protocols.html
	if (ffmpegOptions != nullptr)
	{
		auto options = ffmpegOptions->First();

		while (options->HasCurrent)
		{
			String^ key = options->Current->Key;
			std::wstring keyW(key->Begin());
			std::string keyA(keyW.begin(), keyW.end());
			const char* keyChar = keyA.c_str();

			// Convert value from Object^ to const char*. avformat_open_input will internally convert value from const char* to the correct type
			String^ value = options->Current->Value->ToString();
			std::wstring valueW(value->Begin());
			std::string valueA(valueW.begin(), valueW.end());
			const char* valueChar = valueA.c_str();

			// Add key and value pair entry
			if (av_dict_set(&avDict, keyChar, valueChar, 0) < 0)
			{
				hr = E_INVALIDARG;
				break;
			}

			options->MoveNext();
		}
	}

	return hr;
}

void FFmpegInteropMSS::OnStarting(MediaStreamSource ^sender, MediaStreamSourceStartingEventArgs ^args)
{
	MediaStreamSourceStartingRequest^ request = args->Request;

	// Perform seek operation when MediaStreamSource received seek event from MediaElement
	if (request->StartPosition && request->StartPosition->Value.Duration <= mediaDuration.Duration && (!isFirstSeek || request->StartPosition->Value.Duration > 0))
	{
		auto hr = Seek(request->StartPosition->Value);
		if (SUCCEEDED(hr))
		{
			request->SetActualStartPosition(request->StartPosition->Value);
		}

		if (videoStream && !videoStream->IsEnabled)
		{
			videoStream->EnableStream();
		}

		if (currentAudioStream && !currentAudioStream->IsEnabled)
		{
			currentAudioStream->EnableStream();
		}
	}

	isFirstSeek = false;
}

void FFmpegInteropMSS::OnSampleRequested(Windows::Media::Core::MediaStreamSource ^sender, MediaStreamSourceSampleRequestedEventArgs ^args)
{
	mutexGuard.lock();
	if (mss != nullptr)
	{
		if (currentAudioStream && args->Request->StreamDescriptor == currentAudioStream->StreamDescriptor)
		{
			args->Request->Sample = currentAudioStream->GetNextSample();
		}
		else if (videoStream && args->Request->StreamDescriptor == videoStream->StreamDescriptor)
		{
			args->Request->Sample = videoStream->GetNextSample();
		}
		else
		{
			args->Request->Sample = nullptr;
		}
	}
	mutexGuard.unlock();
}

void FFmpegInteropMSS::OnSwitchStreamsRequested(MediaStreamSource ^ sender, MediaStreamSourceSwitchStreamsRequestedEventArgs ^ args)
{
	mutexGuard.lock();
	if (currentAudioStream && args->Request->OldStreamDescriptor == currentAudioStream->StreamDescriptor)
	{
		if (currentAudioEffects)
		{
			currentAudioStream->DisableFilters();
		}
		currentAudioStream->DisableStream();
		currentAudioStream = nullptr;
	}
	for each (auto stream in audioStreams)
	{
		if (stream->StreamDescriptor == args->Request->NewStreamDescriptor)
		{
			currentAudioStream = stream;
			currentAudioStream->EnableStream();
			if (currentAudioEffects)
			{
				currentAudioStream->SetFilters(currentAudioEffects);
			}
		}
	}
	mutexGuard.unlock();
}

HRESULT FFmpegInteropMSS::Seek(TimeSpan position)
{
	auto hr = S_OK;

	// Select the first valid stream either from video or audio
	int streamIndex = videoStream ? videoStream->StreamIndex : currentAudioStream ? currentAudioStream->StreamIndex : -1;

	if (streamIndex >= 0)
	{
		// Compensate for file start_time, then convert to stream time_base
		int64 correctedPosition;
		if (avFormatCtx->start_time == AV_NOPTS_VALUE)
		{
			correctedPosition = 0;
		}
		else
		{
			correctedPosition = position.Duration + (avFormatCtx->start_time * 10);
		}
		int64_t seekTarget = static_cast<int64_t>(correctedPosition / (av_q2d(avFormatCtx->streams[streamIndex]->time_base) * 10000000));

		if (av_seek_frame(avFormatCtx, streamIndex, seekTarget, AVSEEK_FLAG_BACKWARD) < 0)
		{
			hr = E_FAIL;
			DebugMessage(L" - ### Error while seeking\n");
		}
		else
		{
			// Flush all active streams
			for each (auto stream in sampleProviders)
			{
				if (stream)
				{
					if (stream->IsEnabled)
					{
						stream->Flush();
					}
				}
			}
		}
	}
	else
	{
		hr = E_FAIL;
	}

	return hr;
}

CoreDispatcher ^ FFmpegInterop::FFmpegInteropMSS::GetCurrentDispatcher()
{
	try {
		//try get the current view
		auto wnd = CoreWindow::GetForCurrentThread();
		if (wnd == nullptr)
		{
			wnd = CoreApplication::MainView->CoreWindow;
		}
		if (wnd != nullptr)
			return wnd->Dispatcher;

		return nullptr;
	}
	catch (...)
	{
		return nullptr;
	}

}

// Static function to read file stream and pass data to FFmpeg. Credit to Philipp Sch http://www.codeproject.com/Tips/489450/Creating-Custom-FFmpeg-IO-Context
static int FileStreamRead(void* ptr, uint8_t* buf, int bufSize)
{
	FFmpegInteropMSS^ mss = reinterpret_cast<FFmpegInteropMSS^>(ptr);
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
			auto bom = ((uint32 *)buf)[0];
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
	FFmpegInteropMSS^ mss = reinterpret_cast<FFmpegInteropMSS^>(ptr);
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


