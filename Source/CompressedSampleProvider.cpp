#include "pch.h"

#include <mfapi.h>

#include "CompressedSampleProvider.h"
#include "NativeBufferFactory.h"

using namespace FFmpegInteropX;

CompressedSampleProvider::CompressedSampleProvider(
	FFmpegReader^ reader,
	AVFormatContext* avFormatCtx,
	AVCodecContext* avCodecCtx,
	FFmpegInteropConfig^ config,
	int streamIndex,
	VideoEncodingProperties^ encodingProperties,
	HardwareDecoderStatus hardwareDecoderStatus) :
	MediaSampleProvider(reader, avFormatCtx, avCodecCtx, config, streamIndex, hardwareDecoderStatus),
	videoEncodingProperties(encodingProperties)
{
	decoder = DecoderEngine::SystemDecoder;
}

CompressedSampleProvider::CompressedSampleProvider(
	FFmpegReader^ reader,
	AVFormatContext* avFormatCtx,
	AVCodecContext* avCodecCtx,
	FFmpegInteropConfig^ config,
	int streamIndex,
	AudioEncodingProperties^ encodingProperties,
	HardwareDecoderStatus hardwareDecoderStatus) :
	MediaSampleProvider(reader, avFormatCtx, avCodecCtx, config, streamIndex, hardwareDecoderStatus),
	audioEncodingProperties(encodingProperties)
{
	decoder = DecoderEngine::SystemDecoder;
}


CompressedSampleProvider::CompressedSampleProvider(
	FFmpegReader^ reader,
	AVFormatContext* avFormatCtx,
	AVCodecContext* avCodecCtx,
	FFmpegInteropConfig^ config,
	int streamIndex,
	HardwareDecoderStatus hardwareDecoderStatus) :
	MediaSampleProvider(reader, avFormatCtx, avCodecCtx, config, streamIndex, hardwareDecoderStatus)
{
	decoder = DecoderEngine::SystemDecoder;
}

CompressedSampleProvider::~CompressedSampleProvider()
{
}

HRESULT CompressedSampleProvider::CreateNextSampleBuffer(IBuffer^* pBuffer, int64_t& samplePts, int64_t& sampleDuration, IDirect3DSurface^* surface)
{
	HRESULT hr = S_OK;

	AVPacket* avPacket = NULL;

	hr = GetNextPacket(&avPacket, samplePts, sampleDuration);

	if (hr == S_OK) // Do not create packet at end of stream (S_FALSE)
	{
		hr = CreateBufferFromPacket(avPacket, pBuffer);
	}

	av_packet_free(&avPacket);
	
	return hr;
}

HRESULT CompressedSampleProvider::CreateBufferFromPacket(AVPacket* avPacket, IBuffer^* pBuffer)
{
	HRESULT hr = S_OK;

	// Using direct buffer: just create a buffer reference to hand out to MSS pipeline
	auto bufferRef = av_buffer_ref(avPacket->buf);
	if (bufferRef)
	{
		*pBuffer = NativeBuffer::NativeBufferFactory::CreateNativeBuffer(avPacket->data, avPacket->size, free_buffer, bufferRef);
	}
	else
	{
		hr = E_FAIL;
	}

	return hr;
}

IMediaStreamDescriptor^ CompressedSampleProvider::CreateStreamDescriptor()
{
	IMediaStreamDescriptor^ mediaStreamDescriptor;
	if (videoEncodingProperties != nullptr)
	{
		SetCommonVideoEncodingProperties(videoEncodingProperties, true);
		mediaStreamDescriptor = ref new VideoStreamDescriptor(videoEncodingProperties);
	}
	else
	{
		auto audioStreamDescriptor = ref new AudioStreamDescriptor(audioEncodingProperties);
		if (Windows::Foundation::Metadata::ApiInformation::IsPropertyPresent("Windows.Media.Core.AudioStreamDescriptor", "TrailingEncoderPadding"))
		{
			if (m_pAvStream->codecpar->initial_padding > 0)
			{
				audioStreamDescriptor->LeadingEncoderPadding = (unsigned int)m_pAvStream->codecpar->initial_padding;
			}
			if (m_pAvStream->codecpar->trailing_padding > 0)
			{
				audioStreamDescriptor->TrailingEncoderPadding = (unsigned int)m_pAvStream->codecpar->trailing_padding;
			}
		}
	
		// Set channel layout
		if (m_pAvCodecCtx->channel_layout > 0 && m_pAvCodecCtx->channel_layout < 0x20000000)
		{
			audioEncodingProperties->Properties->Insert(MF_MT_AUDIO_CHANNEL_MASK, (UINT32)m_pAvCodecCtx->channel_layout);
		}

		mediaStreamDescriptor = audioStreamDescriptor;
	}
	return mediaStreamDescriptor;
}
