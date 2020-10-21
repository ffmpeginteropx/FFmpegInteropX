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
#include "UncompressedVideoSampleProvider.h"
#include "NativeBufferFactory.h"
#include <mfapi.h>
#include "VideoEffectFactory.h"

extern "C"
{
#include <libavutil/imgutils.h>
}

using namespace FFmpegInterop;
using namespace NativeBuffer;
using namespace Windows::Media::MediaProperties;

UncompressedVideoSampleProvider::UncompressedVideoSampleProvider(
	FFmpegReader^ reader,
	AVFormatContext* avFormatCtx,
	AVCodecContext* avCodecCtx,
	FFmpegInteropConfig^ config,
	int streamIndex,
	HardwareDecoderStatus hardwareDecoderStatus)
	: UncompressedSampleProvider(reader, avFormatCtx, avCodecCtx, config, streamIndex, hardwareDecoderStatus)
{
}

void UncompressedVideoSampleProvider::SelectOutputFormat()
{
	if (m_config->IsFrameGrabber)
	{
		m_OutputPixelFormat = AV_PIX_FMT_BGRA;
		outputMediaSubtype = MediaEncodingSubtypes::Bgra8;
	}
	else if (m_config->VideoOutputAllowIyuv && (m_pAvCodecCtx->pix_fmt == AV_PIX_FMT_YUV420P || m_pAvCodecCtx->pix_fmt == AV_PIX_FMT_YUVJ420P)
		&& m_pAvCodecCtx->codec->capabilities & AV_CODEC_CAP_DR1)
	{
		// if format is yuv and yuv is allowed and codec supports direct buffer decoding, use yuv
		m_OutputPixelFormat = m_pAvCodecCtx->pix_fmt == AV_PIX_FMT_YUVJ420P ? AV_PIX_FMT_YUVJ420P : AV_PIX_FMT_YUV420P;
		outputMediaSubtype = MediaEncodingSubtypes::Iyuv;
	}
	else if (m_pAvCodecCtx->pix_fmt == AV_PIX_FMT_YUV420P10LE && m_config->VideoOutputAllow10bit)
	{
		m_OutputPixelFormat = AV_PIX_FMT_P010LE;
		OLECHAR* guidString;
		StringFromCLSID(MFVideoFormat_P010, &guidString);

		outputMediaSubtype = ref new String(guidString);

		// ensure memory is freed
		::CoTaskMemFree(guidString);
	}
	else if (m_config->VideoOutputAllowNv12)
	{
		// NV12 is generally the preferred format
		m_OutputPixelFormat = AV_PIX_FMT_NV12;
		outputMediaSubtype = MediaEncodingSubtypes::Nv12;
	}
	else if (m_config->VideoOutputAllowIyuv)
	{
		m_OutputPixelFormat = m_pAvCodecCtx->pix_fmt == AV_PIX_FMT_YUVJ420P ? AV_PIX_FMT_YUVJ420P : AV_PIX_FMT_YUV420P;
		outputMediaSubtype = MediaEncodingSubtypes::Iyuv;
	}
	else if (m_config->VideoOutputAllowBgra8)
	{
		m_OutputPixelFormat = AV_PIX_FMT_BGRA;
		outputMediaSubtype = MediaEncodingSubtypes::Bgra8;
	}
	else // if no format is allowed, we still use NV12
	{
		m_OutputPixelFormat = AV_PIX_FMT_NV12;
		outputMediaSubtype = MediaEncodingSubtypes::Nv12;
	}

	decoderWidth = m_pAvCodecCtx->width;
	decoderHeight = m_pAvCodecCtx->height;

	if (m_OutputPixelFormat != AV_PIX_FMT_BGRA)
	{
		// only BGRA supports unaligned image sizes, all others require 4 pixel alignment
		decoderWidth = ((decoderWidth - 1) / 4 * 4) + 4;
		decoderHeight = ((decoderHeight - 1) / 4 * 4) + 4;
	}

	TargetWidth = decoderWidth;
	TargetHeight = decoderHeight;

	if (m_pAvCodecCtx->pix_fmt == m_OutputPixelFormat && m_pAvCodecCtx->codec->capabilities & AV_CODEC_CAP_DR1)
	{
		// This codec supports direct buffer decoding.
		// Get decoder frame size and override get_buffer2...
		avcodec_align_dimensions(m_pAvCodecCtx, &decoderWidth, &decoderHeight);

		m_pAvCodecCtx->get_buffer2 = get_buffer2;
		m_pAvCodecCtx->opaque = (void*)this;
	}
}

IMediaStreamDescriptor^ UncompressedVideoSampleProvider::CreateStreamDescriptor()
{
	SelectOutputFormat();

	frameProvider = ref new UncompressedFrameProvider(m_pAvFormatCtx, m_pAvCodecCtx, ref new VideoEffectFactory(m_pAvCodecCtx, m_pAvStream));

	auto videoProperties = VideoEncodingProperties::CreateUncompressed(outputMediaSubtype, decoderWidth, decoderHeight);

	SetCommonVideoEncodingProperties(videoProperties, false);

	if (decoderWidth != m_pAvCodecCtx->width || decoderHeight != m_pAvCodecCtx->height)
	{
		MFVideoArea area;
		area.Area.cx = m_pAvCodecCtx->width;
		area.Area.cy = m_pAvCodecCtx->height;
		area.OffsetX.fract = 0;
		area.OffsetX.value = 0;
		area.OffsetY.fract = 0;
		area.OffsetY.value = 0;
		videoProperties->Properties->Insert(MF_MT_MINIMUM_DISPLAY_APERTURE, ref new Array<uint8_t>((byte*)&area, sizeof(MFVideoArea)));
	}

	if (m_OutputPixelFormat == AV_PIX_FMT_YUVJ420P)
	{
		// YUVJ420P uses full range values
		videoProperties->Properties->Insert(MF_MT_VIDEO_NOMINAL_RANGE, (uint32)MFNominalRange_0_255);
	}

	videoProperties->Properties->Insert(MF_MT_INTERLACE_MODE, (uint32)_MFVideoInterlaceMode::MFVideoInterlace_MixedInterlaceOrProgressive);

	return ref new VideoStreamDescriptor(videoProperties);
}

HRESULT UncompressedVideoSampleProvider::InitializeScalerIfRequired(AVFrame* avFrame)
{
	HRESULT hr = S_OK;

	// use bilinear for downscaling, bicubic for upscaling, and point if same resolution
	int scaler =
		avFrame->width == TargetWidth && avFrame->height == TargetHeight ? SWS_POINT :
		avFrame->width < TargetWidth ? SWS_BICUBIC : SWS_BILINEAR;

	// Setup software scaler to convert frame to output pixel type and size
	m_pSwsCtx = sws_getCachedContext(
		m_pSwsCtx,
		avFrame->width,
		avFrame->height,
		(AVPixelFormat)avFrame->format,
		TargetWidth,
		TargetHeight,
		m_OutputPixelFormat,
		scaler,
		NULL,
		NULL,
		NULL);

	if (m_pSwsCtx == nullptr)
	{
		hr = E_OUTOFMEMORY;
	}

	return hr;
}

UncompressedVideoSampleProvider::~UncompressedVideoSampleProvider()
{
	if (m_pSwsCtx)
	{
		sws_freeContext(m_pSwsCtx);
	}

	if (m_pBufferPool)
	{
		av_buffer_pool_uninit(&m_pBufferPool);
	}
}

HRESULT UncompressedVideoSampleProvider::CreateBufferFromFrame(IBuffer^* pBuffer, IDirect3DSurface^* surface, AVFrame* avFrame, int64_t& framePts, int64_t& frameDuration)
{
	HRESULT hr = S_OK;

	if (avFrame->format == m_OutputPixelFormat && avFrame->width == decoderWidth && avFrame->height == decoderHeight)
	{
		// Using direct buffer: just create a buffer reference to hand out to MSS pipeline
		auto bufferRef = av_buffer_ref(avFrame->buf[0]);
		if (bufferRef)
		{
			*pBuffer = NativeBufferFactory::CreateNativeBuffer(bufferRef->data, bufferRef->size, free_buffer, bufferRef);
		}
		else
		{
			hr = E_FAIL;
		}
	}
	else
	{
		hr = InitializeScalerIfRequired(avFrame);

		if (SUCCEEDED(hr))
		{
			// Using scaler: allocate a new frame from buffer pool
			int linesize[4];
			uint8_t* data[4];
			AVBufferRef* buffer;

			hr = FillLinesAndBuffer(linesize, data, &buffer, TargetWidth, TargetHeight);

			if (SUCCEEDED(hr))
			{
				// Convert to output format using FFmpeg software scaler
				auto srcHeight = avFrame->width == m_pAvCodecCtx->width && avFrame->height >= m_pAvCodecCtx->height ? m_pAvCodecCtx->height : avFrame->height;
				if (sws_scale(m_pSwsCtx, (const uint8_t**)(avFrame->data), avFrame->linesize, 0, srcHeight, data, linesize) > 0)
				{
					*pBuffer = NativeBufferFactory::CreateNativeBuffer(buffer->data, buffer->size, free_buffer, buffer);
				}
				else
				{
					free_buffer(buffer);
					hr = E_FAIL;
				}
			}
		}
	}

	// Don't set a timestamp on S_FALSE
	if (hr == S_OK)
	{
		ReadFrameProperties(avFrame, framePts);
	}

	return hr;
}


void FFmpegInterop::UncompressedVideoSampleProvider::ReadFrameProperties(AVFrame* avFrame, int64_t& framePts)
{
	// Try to get the best effort timestamp for the frame.
	if (avFrame->best_effort_timestamp != AV_NOPTS_VALUE)
		framePts = avFrame->best_effort_timestamp;
	m_interlaced_frame = avFrame->interlaced_frame == 1;
	m_top_field_first = avFrame->top_field_first == 1;
	m_chroma_location = avFrame->chroma_location;
	if (m_config->IsFrameGrabber && !IsCleanSample)
	{
		if (m_interlaced_frame)
		{
			// for interlaced content we need to decode two frames to get clean image
			if (!hasFirstInterlacedFrame)
			{
				hasFirstInterlacedFrame = true;
			}
			else
			{
				IsCleanSample = true;
			}
		}
		else
		{
			// for progressive video, we need a key frame or b frame
			IsCleanSample = avFrame->key_frame || avFrame->pict_type == AV_PICTURE_TYPE_B;
		}
	}

	// metadata for jpeg and png is only loaded on frame decode. check for orientation now and apply.
	if (avFrame->metadata && m_pAvCodecCtx->frame_number == 1)
	{
		auto entry = av_dict_get(avFrame->metadata, "Orientation", NULL, 0);
		if (entry)
		{
			auto value = atoi(entry->value);
			uint32 rotationAngle = 0;
			switch (value)
			{
			case 8:
				rotationAngle = 270;
				break;
			case 3:
				rotationAngle = 180;
				break;
			case 6:
				rotationAngle = 90;
				break;
			}
			if (rotationAngle)
			{
				auto videoProperties = ((VideoStreamDescriptor^)this->StreamDescriptor)->EncodingProperties;
				Platform::Guid MF_MT_VIDEO_ROTATION(0xC380465D, 0x2271, 0x428C, 0x9B, 0x83, 0xEC, 0xEA, 0x3B, 0x4A, 0x85, 0xC1);
				videoProperties->Properties->Insert(MF_MT_VIDEO_ROTATION, (uint32)rotationAngle);
			}
		}
	}
}

HRESULT UncompressedVideoSampleProvider::SetSampleProperties(MediaStreamSample^ sample)
{
	if (m_interlaced_frame)
	{
		sample->ExtendedProperties->Insert(MFSampleExtension_Interlaced, TRUE);
		sample->ExtendedProperties->Insert(MFSampleExtension_BottomFieldFirst, m_top_field_first ? safe_cast<Platform::Object^>(FALSE) : TRUE);
		sample->ExtendedProperties->Insert(MFSampleExtension_RepeatFirstField, safe_cast<Platform::Object^>(FALSE));
	}
	else
	{
		sample->ExtendedProperties->Insert(MFSampleExtension_Interlaced, safe_cast<Platform::Object^>(FALSE));
	}

	switch (m_chroma_location)
	{
	case AVCHROMA_LOC_LEFT:
		sample->ExtendedProperties->Insert(MF_MT_VIDEO_CHROMA_SITING, (uint32)MFVideoChromaSubsampling_MPEG2);
		break;
	case AVCHROMA_LOC_CENTER:
		sample->ExtendedProperties->Insert(MF_MT_VIDEO_CHROMA_SITING, (uint32)MFVideoChromaSubsampling_MPEG1);
		break;
	case AVCHROMA_LOC_TOPLEFT:
		if (m_interlaced_frame)
		{
			sample->ExtendedProperties->Insert(MF_MT_VIDEO_CHROMA_SITING, (uint32)MFVideoChromaSubsampling_DV_PAL);
		}
		else
		{
			sample->ExtendedProperties->Insert(MF_MT_VIDEO_CHROMA_SITING, (uint32)MFVideoChromaSubsampling_Cosited);
		}
		break;
	default:
		break;
	}

	return S_OK;
}

HRESULT UncompressedVideoSampleProvider::FillLinesAndBuffer(int* linesize, byte** data, AVBufferRef** buffer, int width, int height)
{
	if (av_image_fill_linesizes(linesize, m_OutputPixelFormat, width) < 0)
	{
		return E_FAIL;
	}

	auto YBufferSize = linesize[0] * height;
	auto UBufferSize = linesize[1] * height / 2;
	auto VBufferSize = linesize[2] * height / 2;
	auto totalSize = YBufferSize + UBufferSize + VBufferSize;

	buffer[0] = AllocateBuffer(totalSize);
	if (!buffer[0])
	{
		return E_OUTOFMEMORY;
	}

	data[0] = buffer[0]->data;
	data[1] = UBufferSize > 0 ? buffer[0]->data + YBufferSize : NULL;
	data[2] = VBufferSize > 0 ? buffer[0]->data + YBufferSize + UBufferSize : NULL;
	data[3] = NULL;

	return S_OK;
}

AVBufferRef* UncompressedVideoSampleProvider::AllocateBuffer(int totalSize)
{
	if (m_config->IsFrameGrabber && TargetBuffer)
	{
		auto bufferRef = av_buffer_create(TargetBuffer, totalSize, [](void*, byte*) {}, NULL, 0);
		return bufferRef;
	}

	if (!m_pBufferPool)
	{
		m_pBufferPool = av_buffer_pool_init(totalSize, NULL);
		if (!m_pBufferPool)
		{
			return NULL;
		}
	}

	auto buffer = av_buffer_pool_get(m_pBufferPool);
	if (!buffer)
	{
		return NULL;
	}
	if (buffer->size != totalSize)
	{
		free_buffer(buffer);
		return NULL;
	}

	return buffer;
}

int UncompressedVideoSampleProvider::get_buffer2(AVCodecContext* avCodecContext, AVFrame* frame, int flags)
{
	// If frame size changes during playback and gets larger than our buffer, we need to switch to sws_scale
	auto provider = reinterpret_cast<UncompressedVideoSampleProvider^>(avCodecContext->opaque);
	bool frameSizeFailure = frame->height > provider->decoderHeight || frame->width > provider->decoderWidth;
	if (frameSizeFailure)
	{
		return avcodec_default_get_buffer2(avCodecContext, frame, flags);
	}
	else
	{
		return provider->FillLinesAndBuffer(frame->linesize, frame->data, frame->buf, provider->decoderWidth, provider->decoderHeight);
	}
}
