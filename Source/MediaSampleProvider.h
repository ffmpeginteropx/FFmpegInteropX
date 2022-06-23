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

    ref class FFmpegReader;
    ref class StreamBuffer;

	ref class MediaSampleProvider abstract
	{
	public:
		virtual ~MediaSampleProvider();
		virtual MediaStreamSample^ GetNextSample();
		virtual void Flush(bool flushBuffers);

		property IMediaStreamDescriptor^ StreamDescriptor
		{
			IMediaStreamDescriptor^ get() { return m_streamDescriptor; }
		}

		property VideoStreamDescriptor^ VideoDescriptor
		{
			VideoStreamDescriptor^ get() { return dynamic_cast<VideoStreamDescriptor^>(m_streamDescriptor); }
		}

		property AudioStreamDescriptor^ AudioDescriptor
		{
			AudioStreamDescriptor^ get() { return dynamic_cast<AudioStreamDescriptor^>(m_streamDescriptor); }
		}

		property IStreamInfo^ StreamInfo
		{
			IStreamInfo^ get() { return streamInfo; }
		}

		property AudioStreamInfo^ AudioInfo
		{
			AudioStreamInfo^ get() { return dynamic_cast<AudioStreamInfo^>(streamInfo); }
		}

		property VideoStreamInfo^ VideoInfo
		{
			VideoStreamInfo^ get() { return dynamic_cast<VideoStreamInfo^>(streamInfo); }
		}

		property SubtitleStreamInfo^ SubtitleInfo
		{
			SubtitleStreamInfo^ get() { return dynamic_cast<SubtitleStreamInfo^>(streamInfo); }
		}

		property int StreamIndex
		{
			int get() { return m_streamIndex; }
		}

		property bool IsEnabled
		{
			bool get() { return m_isEnabled; }
		}

		property HardwareDecoderStatus HardwareAccelerationStatus
		{
			HardwareDecoderStatus get() { return hardwareDecoderStatus; }
		}

		property DecoderEngine Decoder
		{
			DecoderEngine get() { return decoder; }
		}

		property bool IsCleanSample;

		property String^ Name;
		property String^ Language;
		property String^ CodecName;
		property TimeSpan LastSampleTimestamp;

	internal:
		virtual HRESULT Initialize();
		void InitializeNameLanguageCodec();
		virtual void InitializeStreamInfo();
		virtual void QueuePacket(AVPacket* packet);
		HRESULT GetNextPacket(AVPacket** avPacket, LONGLONG& packetPts, LONGLONG& packetDuration);
		virtual HRESULT CreateNextSampleBuffer(IBuffer^* pBuffer, int64_t& samplePts, int64_t& sampleDuration, IDirect3DSurface^* surface) = 0;
		HRESULT GetNextPacketTimestamp(TimeSpan& timestamp, TimeSpan& packetDuration);
		HRESULT SkipPacketsUntilTimestamp(TimeSpan timestamp);
		virtual IMediaStreamDescriptor^ CreateStreamDescriptor() = 0;
		virtual HRESULT SetSampleProperties(MediaStreamSample^ sample) { return S_OK; }; // can be overridded for setting extended properties
        virtual void EnableStream();
        virtual void DisableStream();
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

	protected private:
		MediaSampleProvider(
			FFmpegReader^ reader,
			AVFormatContext* avFormatCtx,
			AVCodecContext* avCodecCtx,
			MediaSourceConfig^ config,
			int streamIndex,
			HardwareDecoderStatus hardwareDecoderStatus);

	private:
        int64 m_nextPacketPts;
        IMediaStreamDescriptor^ m_streamDescriptor;
		HardwareDecoderStatus hardwareDecoderStatus;

	internal:
		// The FFmpeg context. Because they are complex types
		// we declare them as internal so they don't get exposed
		// externally
		MediaSourceConfig^ m_config;
		FFmpegReader^ m_pReader;
        StreamBuffer^ buffer;
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
