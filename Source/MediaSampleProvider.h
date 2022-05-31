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

#pragma once
#include "MediaSourceConfig.h"
#include "TimeSpanHelpers.h"
#include "Enumerations.h"
#include "StreamInfo.h"

namespace FFmpegInteropX
{
	using namespace Windows::Storage::Streams;
	using namespace Windows::Media::Core;
	using namespace Windows::Media::MediaProperties;
	using namespace Windows::Graphics::DirectX::Direct3D11;

	class FFmpegReader;

	class MediaSampleProvider abstract
	{
	public:
		virtual ~MediaSampleProvider();
		virtual MediaStreamSample^ GetNextSample();
		virtual void Flush();

		IMediaStreamDescriptor^ StreamDescriptor()
		{
			return m_streamDescriptor;
		}

		VideoStreamDescriptor^ VideoDescriptor()
		{
			return dynamic_cast<VideoStreamDescriptor^>(m_streamDescriptor);
		}

		AudioStreamDescriptor^ AudioDescriptor()
		{
			return dynamic_cast<AudioStreamDescriptor^>(m_streamDescriptor);
		}

		IStreamInfo^ StreamInfo()
		{
			return streamInfo;
		}

		AudioStreamInfo^ AudioInfo()
		{
			return dynamic_cast<AudioStreamInfo^>(streamInfo);
		}

		VideoStreamInfo^ VideoInfo()
		{
			return dynamic_cast<VideoStreamInfo^>(streamInfo);
		}

		SubtitleStreamInfo^ SubtitleInfo()
		{
			return dynamic_cast<SubtitleStreamInfo^>(streamInfo);
		}

		int StreamIndex()
		{
			return m_streamIndex;
		}

		bool IsEnabled()
		{
			return m_isEnabled;
		}

		HardwareDecoderStatus HardwareAccelerationStatus()
		{
			return hardwareDecoderStatus;
		}

		DecoderEngine Decoder()
		{
			return decoder;
		}

		bool IsCleanSample;

		String^ Name;
		String^ Language;
		String^ CodecName;
		TimeSpan LastSampleTimestamp;

		virtual HRESULT Initialize();
		void InitializeNameLanguageCodec();
		virtual void InitializeStreamInfo();
		virtual void QueuePacket(AVPacket* packet);
		AVPacket* PopPacket();
		HRESULT GetNextPacket(AVPacket** avPacket, LONGLONG& packetPts, LONGLONG& packetDuration);
		virtual HRESULT CreateNextSampleBuffer(IBuffer^* pBuffer, int64_t& samplePts, int64_t& sampleDuration, IDirect3DSurface^* surface) = 0;
		HRESULT GetNextPacketTimestamp(TimeSpan& timestamp, TimeSpan& packetDuration);
		HRESULT SkipPacketsUntilTimestamp(TimeSpan timestamp);
		virtual IMediaStreamDescriptor^ CreateStreamDescriptor() = 0;
		virtual HRESULT SetSampleProperties(MediaStreamSample^ sample) { return S_OK; }; // can be overridded for setting extended properties
		void EnableStream();
		void DisableStream();
		virtual void SetFilters(String^ filterDefinition) { };// override for setting effects in sample providers
		virtual void DisableFilters() {};//override for disabling filters in sample providers;
		virtual void SetCommonVideoEncodingProperties(VideoEncodingProperties^ videoEncodingProperties, bool isCompressedFormat);
		virtual void Detach();
		virtual HRESULT SetHardwareDevice(ID3D11Device* device, ID3D11DeviceContext* context, AVBufferRef* avHardwareContext) { return S_OK; };

		virtual void NotifyCreateSource()
		{
			if (m_pAvFormatCtx->start_time != AV_NOPTS_VALUE)
			{
				auto streamStartTime = ConvertDuration(m_pAvStream->start_time);

				if (m_pAvFormatCtx->start_time == streamStartTime.Duration / 10)
				{
					// use more precise start time
					m_startOffset = streamStartTime.Duration;
				}
				else
				{
					m_startOffset = m_pAvFormatCtx->start_time * 10;
				}
			}
		}

		TimeSpan ConvertPosition(LONGLONG pts)
		{
			return TimeSpan{ LONGLONG(timeBaseFactor * pts) - m_startOffset };
		}

		LONGLONG ConvertPosition(TimeSpan position)
		{
			return LONGLONG((position.Duration + m_startOffset) / timeBaseFactor);
		}

		TimeSpan ConvertDuration(LONGLONG duration)
		{
			return TimeSpan{ LONGLONG(timeBaseFactor * duration) };
		}

		LONGLONG ConvertDuration(TimeSpan duration)
		{
			return LONGLONG(duration.Duration / timeBaseFactor);
		}

	protected:
		MediaSampleProvider(
			std::shared_ptr<FFmpegReader> reader,
			AVFormatContext* avFormatCtx,
			AVCodecContext* avCodecCtx,
			MediaSourceConfig^ config,
			int streamIndex,
			HardwareDecoderStatus hardwareDecoderStatus);

	private:
		std::queue<AVPacket*> m_packetQueue;
		int64 m_nextPacketPts;
		IMediaStreamDescriptor^ m_streamDescriptor;
		HardwareDecoderStatus hardwareDecoderStatus;

	public:
		// The FFmpeg context. Because they are complex types
		// we declare them as internal so they don't get exposed
		// externally
		MediaSourceConfig^ m_config;
		std::shared_ptr<FFmpegReader> m_pReader;
		AVFormatContext* m_pAvFormatCtx;
		AVCodecContext* m_pAvCodecCtx;
		AVStream* m_pAvStream;
		IStreamInfo^ streamInfo;
		bool m_isEnabled = false;
		bool m_isDiscontinuous;
		int m_streamIndex;
		int64 m_startOffset;
		double timeBaseFactor;
		DecoderEngine decoder;
		ID3D11Device* device;
		ID3D11DeviceContext* deviceContext;

	};
}

// free AVBufferRef*
void free_buffer(void* lpVoid);
