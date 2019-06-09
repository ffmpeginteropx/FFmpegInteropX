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
#include "MediaSampleProvider.h"
#include "FFmpegInteropMSS.h"
#include "FFmpegReader.h"
#include "LanguageTagConverter.h"

using namespace FFmpegInterop;
using namespace Windows::Media::MediaProperties;

MediaSampleProvider::MediaSampleProvider(
	FFmpegReader^ reader,
	AVFormatContext* avFormatCtx,
	AVCodecContext* avCodecCtx,
	FFmpegInteropConfig^ config,
	int streamIndex)
	: m_pReader(reader)
	, m_pAvFormatCtx(avFormatCtx)
	, m_pAvCodecCtx(avCodecCtx)
	, m_pAvStream(avFormatCtx->streams[streamIndex])
	, m_config(config)
	, m_streamIndex(streamIndex)
{
	DebugMessage(L"MediaSampleProvider\n");
	if (m_pAvFormatCtx->start_time != AV_NOPTS_VALUE)
	{
		auto streamStartTime = (long long)(av_q2d(m_pAvStream->time_base) * m_pAvStream->start_time * 1000000);

		if (m_pAvFormatCtx->start_time == streamStartTime)
		{
			// calculate more precise start time
			m_startOffset = (long long)(av_q2d(m_pAvStream->time_base) * m_pAvStream->start_time * 10000000);
		}
		else
		{
			m_startOffset = m_pAvFormatCtx->start_time * 10;
		}
	}

	// init first packet pts time from start_time
	if (m_pAvFormatCtx->streams[m_streamIndex]->start_time == AV_NOPTS_VALUE)
	{
		//if start time is AV_NOPTS_VALUE, set it to 0
		m_nextPacketPts = 0;
	}
	else
	{
		//otherwise set the start time of the first packet to the stream start time.
		m_nextPacketPts = m_pAvFormatCtx->streams[m_streamIndex]->start_time;
	}
}

MediaSampleProvider::~MediaSampleProvider()
{
	DebugMessage(L"~MediaSampleProvider\n");

	avcodec_close(m_pAvCodecCtx);
	avcodec_free_context(&m_pAvCodecCtx);
}

HRESULT MediaSampleProvider::Initialize()
{
	m_streamDescriptor = CreateStreamDescriptor();
	if (m_streamDescriptor)
	{
		InitializeNameLanguageCodec();
	}
	return m_streamDescriptor ? S_OK : E_FAIL;
}

void FFmpegInterop::MediaSampleProvider::InitializeNameLanguageCodec()
{
	// unfortunately, setting Name or Language on MediaStreamDescriptor does not have any effect, they are not shown in track selection list
	auto title = av_dict_get(m_pAvStream->metadata, "title", NULL, 0);
	if (title)
	{
		Name = StringUtils::Utf8ToPlatformString(title->value);
	}

	auto language = av_dict_get(m_pAvStream->metadata, "language", NULL, 0);
	if (language)
	{
		Language = StringUtils::Utf8ToPlatformString(language->value);
		if (Language->Length() == 3)
		{
			auto entry = LanguageTagConverter::TryGetLanguage(Language);
			if (entry != nullptr)
			{
				try
				{
					auto winLanguage = ref new Windows::Globalization::Language(entry->TwoLetterCode);
					Language = winLanguage->DisplayName;
				}
				catch (...)
				{
					Language = entry->EnglishName;
				}
			}
		}
		else if (Language->Length() == 2)
		{
			try
			{
				auto winLanguage = ref new Windows::Globalization::Language(Language);
				Language = winLanguage->DisplayName;
			}
			catch (...)
			{
			}
		}

		if (!Name)
		{
			Name = Language;
		}
	}

	if (!Name && (m_pAvStream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO || m_pAvStream->codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE))
	{
		int count = 0;
		int number = 0;
		for (unsigned int i = 0; i < m_pAvFormatCtx->nb_streams; i++)
		{
			if (m_pAvFormatCtx->streams[i]->codecpar->codec_type == m_pAvStream->codecpar->codec_type)
			{
				count++;
				if (i == StreamIndex)
				{
					number = count;
				}
			}
		}

		String^ name = m_pAvStream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO ? m_config->DefaultAudioStreamName : m_config->DefaultSubtitleStreamName;
		if (count > 1)
		{
			name = name + " " + number.ToString();
		}
		Name = name;
	}

	auto codec = m_pAvCodecCtx->codec_descriptor->name;
	if (codec)
	{
		CodecName = StringUtils::Utf8ToPlatformString(codec);
	}
}

MediaStreamSample^ MediaSampleProvider::GetNextSample()
{
	DebugMessage(L"GetNextSample\n");

	HRESULT hr = S_OK;

	MediaStreamSample^ sample;
	if (m_isEnabled)
	{
		IBuffer^ buffer = nullptr;
		LONGLONG pts = 0;
		LONGLONG dur = 0;

		hr = CreateNextSampleBuffer(&buffer, pts, dur);
		
		if (hr == S_OK)
		{
			pts = LONGLONG(av_q2d(m_pAvStream->time_base) * 10000000 * pts) - m_startOffset;
			dur = LONGLONG(av_q2d(m_pAvStream->time_base) * 10000000 * dur);

			TimeSpan duration = { dur };
			sample = MediaStreamSample::CreateFromBuffer(buffer, { pts });
			sample->Duration = duration;
			sample->Discontinuous = m_isDiscontinuous;

			hr = SetSampleProperties(sample);

			m_isDiscontinuous = false;
		}
		else if (hr == S_FALSE)
		{
			DebugMessage(L"End of stream reached.\n");
			DisableStream();
		}
		else
		{
			DebugMessage(L"Error reading next packet.\n");
			DisableStream();
		}
	}

	return sample;
}

HRESULT MediaSampleProvider::GetNextPacket(AVPacket** avPacket, LONGLONG & packetPts, LONGLONG & packetDuration)
{
	HRESULT hr = S_OK;

	// Continue reading until there is an appropriate packet in the stream
	while (m_packetQueue.empty())
	{
		if (m_pReader->ReadPacket() < 0)
		{
			DebugMessage(L"GetNextSample reaching EOF\n");
			break;
		}
	}

	if (!m_packetQueue.empty())
	{
		// read next packet and set pts values
		auto packet = PopPacket();
		*avPacket = packet;

		packetDuration = packet->duration;
		
		if (packet->pts != AV_NOPTS_VALUE)
		{
			packetPts = packet->pts;
			// Set the PTS for the next sample if it doesn't one.
			m_nextPacketPts = packetPts + packetDuration;
		}
		else
		{
			packetPts = m_nextPacketPts;
			// Set the PTS for the next sample if it doesn't one.
			m_nextPacketPts += packetDuration;
		}
	}
	else
	{
		hr = S_FALSE;
	}

	return hr;
}


void MediaSampleProvider::QueuePacket(AVPacket *packet)
{
	DebugMessage(L" - QueuePacket\n");

	if (m_isEnabled)
	{
		m_packetQueue.push(packet);
	}
	else
	{
		av_packet_free(&packet);
	}
}

AVPacket* MediaSampleProvider::PopPacket()
{
	DebugMessage(L" - PopPacket\n");
	AVPacket* result = NULL;

	if (!m_packetQueue.empty())
	{
		result = m_packetQueue.front();
		m_packetQueue.pop();
	}

	return result;
}

void MediaSampleProvider::Flush()
{
	DebugMessage(L"Flush\n");
	while (!m_packetQueue.empty())
	{
		AVPacket *avPacket = PopPacket();
		av_packet_free(&avPacket);
	}
	if (m_pAvCodecCtx)
	{
		avcodec_flush_buffers(m_pAvCodecCtx);
	}
	m_isDiscontinuous = true;
	IsCleanSample = false;
}

void MediaSampleProvider::EnableStream()
{
	DebugMessage(L"EnableStream\n");
	m_isEnabled = true;
}

void MediaSampleProvider::DisableStream()
{
	DebugMessage(L"DisableStream\n");
	Flush();
	m_isEnabled = false;
}

void MediaSampleProvider::SetCommonVideoEncodingProperties(VideoEncodingProperties^ videoProperties, bool isCompressedFormat)
{
	if (isCompressedFormat)
	{
		videoProperties->Width = m_pAvCodecCtx->width;
		videoProperties->Height = m_pAvCodecCtx->height;
		videoProperties->ProfileId = m_pAvCodecCtx->profile;
	}

	if (m_pAvCodecCtx->sample_aspect_ratio.num > 0 &&
		m_pAvCodecCtx->sample_aspect_ratio.den > 0 &&
		m_pAvCodecCtx->sample_aspect_ratio.num != m_pAvCodecCtx->sample_aspect_ratio.den)
	{
		videoProperties->PixelAspectRatio->Numerator = m_pAvCodecCtx->sample_aspect_ratio.num;
		videoProperties->PixelAspectRatio->Denominator = m_pAvCodecCtx->sample_aspect_ratio.den;
	}
	else
	{
		videoProperties->PixelAspectRatio->Numerator = 1;
		videoProperties->PixelAspectRatio->Denominator = 1;
	}

	// set video rotation
	bool rotateVideo = false;
	int rotationAngle;
	AVDictionaryEntry *rotate_tag = av_dict_get(m_pAvStream->metadata, "rotate", NULL, 0);
	if (rotate_tag != NULL)
	{
		rotateVideo = true;
		rotationAngle = atoi(rotate_tag->value);
	}
	else
	{
		rotateVideo = false;
	}
	if (rotateVideo)
	{
		Platform::Guid MF_MT_VIDEO_ROTATION(0xC380465D, 0x2271, 0x428C, 0x9B, 0x83, 0xEC, 0xEA, 0x3B, 0x4A, 0x85, 0xC1);
		videoProperties->Properties->Insert(MF_MT_VIDEO_ROTATION, (uint32)rotationAngle);
	}

	// Detect the correct framerate
	if (m_pAvCodecCtx->framerate.num != 0 || m_pAvCodecCtx->framerate.den != 1)
	{
		videoProperties->FrameRate->Numerator = m_pAvCodecCtx->framerate.num;
		videoProperties->FrameRate->Denominator = m_pAvCodecCtx->framerate.den;
	}
	else if (m_pAvStream->avg_frame_rate.num != 0 || m_pAvStream->avg_frame_rate.den != 0)
	{
		videoProperties->FrameRate->Numerator = m_pAvStream->avg_frame_rate.num;
		videoProperties->FrameRate->Denominator = m_pAvStream->avg_frame_rate.den;
	}

	videoProperties->Bitrate = (unsigned int)m_pAvCodecCtx->bit_rate;
}

void MediaSampleProvider::Detach()
{
	Flush();
	m_pReader = nullptr;
	avcodec_close(m_pAvCodecCtx);
	avcodec_free_context(&m_pAvCodecCtx);
}

void free_buffer(void *lpVoid)
{
	auto buffer = (AVBufferRef *)lpVoid;
	av_buffer_unref(&buffer);
}