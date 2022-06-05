#include "pch.h"
#include "CompressedSampleProvider.h"
#include "NativeBufferFactory.h"

using namespace FFmpegInteropX;

CompressedSampleProvider::CompressedSampleProvider(
	std::shared_ptr<FFmpegReader> reader,
	AVFormatContext* avFormatCtx,
	AVCodecContext* avCodecCtx,
	MediaSourceConfig config,
	int streamIndex,
	winrt::Windows::Media::MediaProperties::VideoEncodingProperties encodingProperties,
	HardwareDecoderStatus hardwareDecoderStatus) :
	MediaSampleProvider(reader, avFormatCtx, avCodecCtx, config, streamIndex, hardwareDecoderStatus),
	videoEncodingProperties(encodingProperties)
{
	decoder = DecoderEngine::SystemDecoder;
}

CompressedSampleProvider::CompressedSampleProvider(
	std::shared_ptr<FFmpegReader> reader,
	AVFormatContext* avFormatCtx,
	AVCodecContext* avCodecCtx,
	MediaSourceConfig config,
	int streamIndex,
	winrt::Windows::Media::MediaProperties::AudioEncodingProperties encodingProperties,
	HardwareDecoderStatus hardwareDecoderStatus) :
	MediaSampleProvider(reader, avFormatCtx, avCodecCtx, config, streamIndex, hardwareDecoderStatus),
	audioEncodingProperties(encodingProperties)
{
	decoder = DecoderEngine::SystemDecoder;
}


CompressedSampleProvider::CompressedSampleProvider(
	std::shared_ptr<FFmpegReader> reader,
	AVFormatContext* avFormatCtx,
	AVCodecContext* avCodecCtx,
	MediaSourceConfig config,
	int streamIndex,
	HardwareDecoderStatus hardwareDecoderStatus) :
	MediaSampleProvider(reader, avFormatCtx, avCodecCtx, config, streamIndex, hardwareDecoderStatus)
{
	decoder = DecoderEngine::SystemDecoder;
}

CompressedSampleProvider::~CompressedSampleProvider()
{
}

HRESULT CompressedSampleProvider::CreateNextSampleBuffer(winrt::Windows::Storage::Streams::IBuffer* pBuffer, int64_t& samplePts, int64_t& sampleDuration, winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DSurface* surface)
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

HRESULT CompressedSampleProvider::CreateBufferFromPacket(AVPacket* avPacket, winrt::Windows::Storage::Streams::IBuffer* pBuffer)
{
	HRESULT hr = S_OK;

	// Using direct buffer: just create a buffer reference to hand out to MSS pipeline
	auto bufferRef = av_buffer_ref(avPacket->buf);
	if (bufferRef)
	{
		*pBuffer = *NativeBuffer::NativeBufferFactory::CreateNativeBuffer(avPacket->data, avPacket->size, free_buffer, bufferRef);
	}
	else
	{
		hr = E_FAIL;
	}

	return hr;
}

winrt::Windows::Media::Core::IMediaStreamDescriptor CompressedSampleProvider::CreateStreamDescriptor()
{
	winrt::Windows::Media::Core::IMediaStreamDescriptor mediaStreamDescriptor;
	if (videoEncodingProperties != nullptr)
	{
		SetCommonVideoEncodingProperties(videoEncodingProperties, true);
		mediaStreamDescriptor = winrt::Windows::Media::Core::VideoStreamDescriptor(videoEncodingProperties);
	}
	else
	{
		auto audioStreamDescriptor = winrt::Windows::Media::Core::AudioStreamDescriptor(audioEncodingProperties);
		if (winrt::Windows::Foundation::Metadata::ApiInformation::IsPropertyPresent(L"Windows.Media.Core.AudioStreamDescriptor", L"TrailingEncoderPadding"))
		{
			if (m_pAvStream->codecpar->initial_padding > 0)
			{
				audioStreamDescriptor.LeadingEncoderPadding((unsigned int)m_pAvStream->codecpar->initial_padding);
			}
			if (m_pAvStream->codecpar->trailing_padding > 0)
			{
				audioStreamDescriptor.TrailingEncoderPadding((unsigned int)m_pAvStream->codecpar->trailing_padding);
			}
		}
	
		// Set channel layout
		if (m_pAvCodecCtx->channel_layout > 0 && m_pAvCodecCtx->channel_layout < 0x20000000)
		{
			audioEncodingProperties.Properties().Insert(MF_MT_AUDIO_CHANNEL_MASK, winrt::box_value((UINT32)m_pAvCodecCtx->channel_layout));
		}

		mediaStreamDescriptor = audioStreamDescriptor;
	}
	return mediaStreamDescriptor;
}
