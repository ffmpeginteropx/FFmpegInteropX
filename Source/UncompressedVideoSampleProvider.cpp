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
#include <libavutil/mastering_display_metadata.h>
}

using namespace FFmpegInteropX;
using namespace NativeBuffer;
using namespace Windows::Media::MediaProperties;

UncompressedVideoSampleProvider::UncompressedVideoSampleProvider(
	FFmpegReader^ reader,
	AVFormatContext* avFormatCtx,
	AVCodecContext* avCodecCtx,
	MediaSourceConfig^ config,
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
		&& m_pAvCodecCtx->codec->capabilities & AV_CODEC_CAP_DR1 && decoder != DecoderEngine::FFmpegD3D11HardwareDecoder)
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

	outputWidth = outputFrameWidth = m_pAvCodecCtx->width;
	outputHeight = outputFrameHeight = m_pAvCodecCtx->height;

	if (m_OutputPixelFormat != AV_PIX_FMT_BGRA)
	{
		// only BGRA supports unaligned image sizes, all others require 4 pixel alignment
		outputFrameWidth = ((outputWidth - 1) / 4 * 4) + 4;
		outputFrameHeight = ((outputHeight - 1) / 4 * 4) + 4;
	}

	if (m_pAvCodecCtx->pix_fmt == m_OutputPixelFormat && m_pAvCodecCtx->codec->capabilities & AV_CODEC_CAP_DR1)
	{
		// This codec supports direct buffer decoding.

		m_pAvCodecCtx->get_buffer2 = get_buffer2;
		m_pAvCodecCtx->opaque = (void*)this;
	}
}

IMediaStreamDescriptor^ UncompressedVideoSampleProvider::CreateStreamDescriptor()
{
	SelectOutputFormat();

	frameProvider = ref new UncompressedFrameProvider(m_pAvFormatCtx, m_pAvCodecCtx, ref new VideoEffectFactory(m_pAvCodecCtx, m_pAvStream));

	auto videoProperties = VideoEncodingProperties::CreateUncompressed(outputMediaSubtype, outputFrameWidth, outputFrameHeight);
	auto properties = videoProperties->Properties;
	auto codecPar = m_pAvStream->codecpar;

	SetCommonVideoEncodingProperties(videoProperties, false);

	MFVideoArea area;
	area.Area.cx = outputWidth;
	area.Area.cy = outputHeight;
	area.OffsetX.fract = 0;
	area.OffsetX.value = 0;
	area.OffsetY.fract = 0;
	area.OffsetY.value = 0;
	properties->Insert(MF_MT_MINIMUM_DISPLAY_APERTURE, ref new Array<uint8_t>((byte*)&area, sizeof(MFVideoArea)));

	if (codecPar->color_primaries != AVCOL_PRI_UNSPECIFIED)
	{
		MFVideoPrimaries videoPrimaries{ MFVideoPrimaries_Unknown };
		switch (codecPar->color_primaries)
		{
		case AVCOL_PRI_RESERVED0:
		case AVCOL_PRI_RESERVED:
			videoPrimaries = MFVideoPrimaries_reserved;
			break;

		case AVCOL_PRI_BT709:
			videoPrimaries = MFVideoPrimaries_BT709;
			break;

		case  AVCOL_PRI_BT470M:
			videoPrimaries = MFVideoPrimaries_BT470_2_SysM;
			break;

		case AVCOL_PRI_BT470BG:
			videoPrimaries = MFVideoPrimaries_BT470_2_SysBG;
			break;

		case AVCOL_PRI_SMPTE170M:
			videoPrimaries = MFVideoPrimaries_SMPTE170M;
			break;

		case AVCOL_PRI_SMPTE240M:
			videoPrimaries = MFVideoPrimaries_SMPTE240M;
			break;

		case AVCOL_PRI_FILM:
			videoPrimaries = MFVideoPrimaries_SMPTE_C;
			break;

		case AVCOL_PRI_BT2020:
			videoPrimaries = MFVideoPrimaries_BT2020;
			break;

		default:
			break;
		}

		properties->Insert(MF_MT_VIDEO_PRIMARIES, PropertyValue::CreateUInt32(videoPrimaries));
	}

	if (codecPar->color_trc != AVCOL_TRC_UNSPECIFIED)
	{
		MFVideoTransferFunction videoTransferFunc{ MFVideoTransFunc_Unknown };
		switch (codecPar->color_trc)
		{
		case AVCOL_TRC_BT709:
		case AVCOL_TRC_GAMMA22:
		case AVCOL_TRC_SMPTE170M:
			videoTransferFunc = MFVideoTransFunc_22;
			break;

		case AVCOL_TRC_GAMMA28:
			videoTransferFunc = MFVideoTransFunc_28;
			break;

		case AVCOL_TRC_SMPTE240M:
			videoTransferFunc = MFVideoTransFunc_240M;
			break;

		case AVCOL_TRC_LINEAR:
			videoTransferFunc = MFVideoTransFunc_10;
			break;

		case AVCOL_TRC_LOG:
			videoTransferFunc = MFVideoTransFunc_Log_100;
			break;

		case AVCOL_TRC_LOG_SQRT:
			videoTransferFunc = MFVideoTransFunc_Log_316;
			break;

		case AVCOL_TRC_BT1361_ECG:
			videoTransferFunc = MFVideoTransFunc_709;
			break;

		case AVCOL_TRC_BT2020_10:
		case AVCOL_TRC_BT2020_12:
			videoTransferFunc = MFVideoTransFunc_2020;
			break;

		case AVCOL_TRC_SMPTEST2084:
			videoTransferFunc = MFVideoTransFunc_2084;
			break;

		case AVCOL_TRC_ARIB_STD_B67:
			videoTransferFunc = MFVideoTransFunc_HLG;
			break;

		default:
			break;
		}

		properties->Insert(MF_MT_TRANSFER_FUNCTION, PropertyValue::CreateUInt32(videoTransferFunc));
	}

	if (codecPar->color_range != AVCOL_RANGE_UNSPECIFIED)
	{
		MFNominalRange nominalRange{ codecPar->color_range == AVCOL_RANGE_JPEG ? MFNominalRange_0_255 : MFNominalRange_16_235 };
		properties->Insert(MF_MT_VIDEO_NOMINAL_RANGE, PropertyValue::CreateUInt32(nominalRange));
	}

	AVContentLightMetadata* contentLightMetadata{ reinterpret_cast<AVContentLightMetadata*>(av_stream_get_side_data(m_pAvStream, AV_PKT_DATA_CONTENT_LIGHT_LEVEL, nullptr)) };
	if (contentLightMetadata != nullptr)
	{
		properties->Insert(MF_MT_MAX_LUMINANCE_LEVEL, PropertyValue::CreateUInt32(contentLightMetadata->MaxCLL));
		properties->Insert(MF_MT_MAX_FRAME_AVERAGE_LUMINANCE_LEVEL, PropertyValue::CreateUInt32(contentLightMetadata->MaxFALL));
	}

	AVMasteringDisplayMetadata* masteringDisplayMetadata{ reinterpret_cast<AVMasteringDisplayMetadata*>(av_stream_get_side_data(m_pAvStream, AV_PKT_DATA_MASTERING_DISPLAY_METADATA, nullptr)) };
	if (masteringDisplayMetadata != nullptr)
	{
		if (masteringDisplayMetadata->has_luminance)
		{
			constexpr uint32_t MASTERING_DISP_LUMINANCE_SCALE{ 10000 };
			properties->Insert(MF_MT_MIN_MASTERING_LUMINANCE, PropertyValue::CreateUInt32(static_cast<uint32_t>(MASTERING_DISP_LUMINANCE_SCALE * av_q2d(masteringDisplayMetadata->min_luminance))));
			properties->Insert(MF_MT_MAX_MASTERING_LUMINANCE, PropertyValue::CreateUInt32(static_cast<uint32_t>(av_q2d(masteringDisplayMetadata->max_luminance))));
		}

		if (masteringDisplayMetadata->has_primaries)
		{
			MT_CUSTOM_VIDEO_PRIMARIES customVideoPrimaries
			{
				static_cast<float>(av_q2d(masteringDisplayMetadata->display_primaries[0][0])),
				static_cast<float>(av_q2d(masteringDisplayMetadata->display_primaries[0][1])),
				static_cast<float>(av_q2d(masteringDisplayMetadata->display_primaries[1][0])),
				static_cast<float>(av_q2d(masteringDisplayMetadata->display_primaries[1][1])),
				static_cast<float>(av_q2d(masteringDisplayMetadata->display_primaries[2][0])),
				static_cast<float>(av_q2d(masteringDisplayMetadata->display_primaries[2][1])),
				static_cast<float>(av_q2d(masteringDisplayMetadata->white_point[0])),
				static_cast<float>(av_q2d(masteringDisplayMetadata->white_point[1]))
			};
			auto data = Platform::ArrayReference<uint8_t>(reinterpret_cast<uint8_t*>(&customVideoPrimaries), sizeof(customVideoPrimaries));
			properties->Insert(MF_MT_CUSTOM_VIDEO_PRIMARIES, PropertyValue::CreateUInt8Array(data));
		}
	}

	videoProperties->Properties->Insert(MF_MT_INTERLACE_MODE, (uint32)_MFVideoInterlaceMode::MFVideoInterlace_MixedInterlaceOrProgressive);

	return ref new VideoStreamDescriptor(videoProperties);
}

HRESULT UncompressedVideoSampleProvider::InitializeScalerIfRequired(AVFrame* avFrame)
{
	HRESULT hr = S_OK;

	// use bilinear for downscaling, bicubic for upscaling.
	// don't use point scaling since format conversions might involve scaling even if resolution is same (chroma subsampling)
	int scaler = avFrame->width < outputWidth ? SWS_BICUBIC : SWS_BILINEAR;

	// Setup software scaler to convert frame to output pixel type and size
	m_pSwsCtx = sws_getCachedContext(
		m_pSwsCtx,
		avFrame->width,
		avFrame->height,
		(AVPixelFormat)avFrame->format,
		outputWidth,
		outputHeight,
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

	if (sourceBufferPool)
	{
		av_buffer_pool_uninit(&sourceBufferPool);
	}

	if (targetBufferPool)
	{
		av_buffer_pool_uninit(&targetBufferPool);
	}
}

HRESULT UncompressedVideoSampleProvider::CreateBufferFromFrame(IBuffer^* pBuffer, IDirect3DSurface^* surface, AVFrame* avFrame, int64_t& framePts, int64_t& frameDuration)
{
	HRESULT hr = S_OK;
	CheckFrameSize(avFrame);

	if (outputDirectBuffer)
	{
		// Using direct buffer: just create a buffer reference to hand out to MSS pipeline
		auto bufferRef = av_buffer_ref(avFrame->buf[0]);
		if (bufferRef)
		{
			*pBuffer = NativeBufferFactory::CreateNativeBuffer(bufferRef->data, (UINT32)bufferRef->size, free_buffer, bufferRef);
		}
		else
		{
			hr = E_FAIL;
		}
	}
	else if (avFrame->format == m_OutputPixelFormat && avFrame->width == outputWidth && avFrame->height == outputHeight)
	{
		// Same format and size but non-contiguous buffer: Copy to contiguous buffer
		int linesize[4];
		uint8_t* data[4];
		AVBufferRef* buffer;

		hr = FillLinesAndBuffer(linesize, data, &buffer, outputFrameWidth, outputFrameHeight, false);

		if (SUCCEEDED(hr))
		{
			av_image_copy(data, linesize, (const uint8_t**)avFrame->data, avFrame->linesize, m_OutputPixelFormat, outputWidth, outputHeight);
			*pBuffer = NativeBufferFactory::CreateNativeBuffer(buffer->data, (UINT32)buffer->size, free_buffer, buffer);
		}
	}
	else
	{
		// Different format or size: Use scaler
		hr = InitializeScalerIfRequired(avFrame);

		if (SUCCEEDED(hr))
		{
			int linesize[4];
			uint8_t* data[4];
			AVBufferRef* buffer;

			hr = FillLinesAndBuffer(linesize, data, &buffer, outputFrameWidth, outputFrameHeight, false);

			if (SUCCEEDED(hr))
			{
				// Convert to output format using FFmpeg software scaler
				if (sws_scale(m_pSwsCtx, (const uint8_t**)(avFrame->data), avFrame->linesize, 0, avFrame->height, data, linesize) > 0)
				{
					*pBuffer = NativeBufferFactory::CreateNativeBuffer(buffer->data, (UINT32)buffer->size, free_buffer, buffer);
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


void FFmpegInteropX::UncompressedVideoSampleProvider::ReadFrameProperties(AVFrame* avFrame, int64_t& framePts)
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

	auto sideData = av_frame_get_side_data(avFrame, AV_FRAME_DATA_CONTENT_LIGHT_LEVEL);
	if (sideData)
	{
		auto contentLightMetadata = reinterpret_cast<AVContentLightMetadata*>(sideData->data);
		maxCLL = PropertyValue::CreateUInt32(contentLightMetadata->MaxCLL);
		maxFALL = PropertyValue::CreateUInt32(contentLightMetadata->MaxFALL);
	}
	else
	{
		maxCLL = maxFALL = nullptr;
	}

	sideData = av_frame_get_side_data(avFrame, AV_FRAME_DATA_MASTERING_DISPLAY_METADATA);
	if (sideData)
	{
		auto masteringDisplayMetadata = reinterpret_cast<AVMasteringDisplayMetadata*>(sideData->data);
		if (masteringDisplayMetadata->has_luminance)
		{
			constexpr uint32_t MASTERING_DISP_LUMINANCE_SCALE{ 10000 };
			minLuminance = PropertyValue::CreateUInt32(static_cast<uint32_t>(MASTERING_DISP_LUMINANCE_SCALE * av_q2d(masteringDisplayMetadata->min_luminance)));
			maxLuminance = PropertyValue::CreateUInt32(static_cast<uint32_t>(av_q2d(masteringDisplayMetadata->max_luminance)));
		}
		else
		{
			minLuminance = maxLuminance = nullptr;
		}

		if (masteringDisplayMetadata->has_primaries)
		{
			MT_CUSTOM_VIDEO_PRIMARIES customVideoPrimaries
			{
				static_cast<float>(av_q2d(masteringDisplayMetadata->display_primaries[0][0])),
				static_cast<float>(av_q2d(masteringDisplayMetadata->display_primaries[0][1])),
				static_cast<float>(av_q2d(masteringDisplayMetadata->display_primaries[1][0])),
				static_cast<float>(av_q2d(masteringDisplayMetadata->display_primaries[1][1])),
				static_cast<float>(av_q2d(masteringDisplayMetadata->display_primaries[2][0])),
				static_cast<float>(av_q2d(masteringDisplayMetadata->display_primaries[2][1])),
				static_cast<float>(av_q2d(masteringDisplayMetadata->white_point[0])),
				static_cast<float>(av_q2d(masteringDisplayMetadata->white_point[1]))
			};
			auto data = Platform::ArrayReference<uint8_t>(reinterpret_cast<uint8_t*>(&customVideoPrimaries), sizeof(customVideoPrimaries));
			customPrimaries = PropertyValue::CreateUInt8Array(data);
		}
		else
		{
			customPrimaries = nullptr;
		}
	}
	else
	{
		minLuminance = maxLuminance = customPrimaries = nullptr;
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

	if (maxCLL && maxFALL)
	{
		sample->ExtendedProperties->Insert(MF_MT_MAX_LUMINANCE_LEVEL, maxCLL);
		sample->ExtendedProperties->Insert(MF_MT_MAX_FRAME_AVERAGE_LUMINANCE_LEVEL, maxFALL);
	}

	if (minLuminance && maxLuminance)
	{
		sample->ExtendedProperties->Insert(MF_MT_MIN_MASTERING_LUMINANCE, minLuminance);
		sample->ExtendedProperties->Insert(MF_MT_MAX_MASTERING_LUMINANCE, maxLuminance);
	}

	if (customPrimaries)
	{
		sample->ExtendedProperties->Insert(MF_MT_CUSTOM_VIDEO_PRIMARIES, customPrimaries);
	}

	return S_OK;
}

HRESULT UncompressedVideoSampleProvider::FillLinesAndBuffer(int* linesize, byte** data, AVBufferRef** buffer, int width, int height, bool isSourceBuffer)
{
	// this method more or less follows the ffmpeg av_image_alloc() implementation

	ptrdiff_t linesizes1[4];
	size_t sizes[4];
	int totalSize = 0;

	if (av_image_check_size(width, height, 0, NULL) < 0)
	{
		return E_FAIL;
	}

	if (av_image_fill_linesizes(linesize, m_OutputPixelFormat, width) < 0)
	{
		return E_FAIL;
	}
	
	for (int i = 0; i < 4; i++) {
		linesizes1[i] = linesize[i];
	}

	if (av_image_fill_plane_sizes(sizes, m_OutputPixelFormat, height, linesizes1) < 0)
	{
		return E_FAIL;
	}

	for (int i = 0; i < 4; i++) {
		if (sizes[i] > INT_MAX - totalSize)
			return AVERROR(EINVAL);
		totalSize += (int)sizes[i];
	}

	// allocate buffer
	buffer[0] = AllocateBuffer(totalSize, isSourceBuffer ? &sourceBufferPool : &targetBufferPool, isSourceBuffer ? &sourceBufferPoolSize : &targetBufferPoolSize);
	if (!buffer[0])
	{
		return E_OUTOFMEMORY;
	}

	// fill pointers
	if (av_image_fill_pointers(data, m_OutputPixelFormat, height, buffer[0]->data, linesize) < 0)
	{
		return E_FAIL;
	}

	return S_OK;
}

AVBufferRef* UncompressedVideoSampleProvider::AllocateBuffer(int requestedSize, AVBufferPool** bufferPool, int* bufferPoolSize)
{
	if (m_config->IsFrameGrabber && TargetBuffer)
	{
		auto bufferRef = av_buffer_create(TargetBuffer, requestedSize, [](void*, byte*) {}, NULL, 0);
		return bufferRef;
	}

	if (*bufferPool && *bufferPoolSize != requestedSize)
	{
		av_buffer_pool_uninit(bufferPool);
	}

	if (!*bufferPool)
	{
		*bufferPool = av_buffer_pool_init(requestedSize, NULL);
		if (!*bufferPool)
		{
			return NULL;
		}
		*bufferPoolSize = requestedSize;
	}

	auto buffer = av_buffer_pool_get(*bufferPool);
	if (!buffer)
	{
		return NULL;
	}

	return buffer;
}

int UncompressedVideoSampleProvider::get_buffer2(AVCodecContext* avCodecContext, AVFrame* frame, int flags)
{
	auto provider = reinterpret_cast<UncompressedVideoSampleProvider^>(avCodecContext->opaque);

	if (frame->format == AV_PIX_FMT_D3D11 || frame->format == AV_PIX_FMT_D3D11VA_VLD)
	{
		// custom buffer allocation not supported for HW formats. switch to default function.
		avCodecContext->get_buffer2 = avcodec_default_get_buffer2;
		return avcodec_default_get_buffer2(avCodecContext, frame, flags);
	}
	else
	{
		return provider->FillLinesAndBuffer(frame->linesize, frame->data, frame->buf, frame->width, frame->height, true);
	}
}


void FFmpegInteropX::UncompressedVideoSampleProvider::CheckFrameSize(AVFrame* avFrame)
{
	outputDirectBuffer = true;
	int frameWidth = avFrame->width;
	int frameHeight = avFrame->height;

	// check if format has contiguous buffer for direct buffer approach
	if (avFrame->format == m_OutputPixelFormat && avFrame->buf && avFrame->buf[0] && avFrame->data && avFrame->linesize)
	{
		auto firstBufferStart = avFrame->buf[0]->data;
		auto firstBufferEnd = firstBufferStart + avFrame->buf[0]->size;
		for (int i = 0; i < 4; i++)
		{
			if (avFrame->linesize[i] && !(avFrame->data[i] >= firstBufferStart && avFrame->data[i] <= firstBufferEnd))
			{
				outputDirectBuffer = false;
			}
		}
	}
	else
	{
		outputDirectBuffer = false;
	}

	// if format has more than one plane, calculate true contiguous frame height
	if (outputDirectBuffer && avFrame->linesize[1])
	{
		auto firstPlaneSize = avFrame->data[1] - avFrame->data[0];
		double calculatedFrameHeight = (double)firstPlaneSize / avFrame->linesize[0];
		if (calculatedFrameHeight == (int)calculatedFrameHeight)
		{
			frameHeight = (int)calculatedFrameHeight;
		}
		else
		{
			outputDirectBuffer = false;
		}
	}

	if (m_OutputPixelFormat != AV_PIX_FMT_BGRA)
	{
		// only BGRA supports unaligned image sizes, all others require 4 pixel alignment
		int frameWidthAligned = ((frameWidth - 1) / 4 * 4) + 4;
		int frameHeightAligned = ((frameHeight - 1) / 4 * 4) + 4;

		if (frameWidth != frameWidthAligned)
		{
			frameWidth = frameWidthAligned;
			outputDirectBuffer = false;
		}
		if (frameHeight != frameHeightAligned)
		{
			frameHeight = frameHeightAligned;
			outputDirectBuffer = false;
		}
	}

	bool hasFormatChanged = avFrame->width != outputWidth || avFrame->height != outputHeight ||
		frameWidth != outputFrameWidth || frameHeight != outputFrameHeight;

	bool isFrameGrabberOverride = TargetWidth > 0 && TargetHeight > 0;

	if (isFrameGrabberOverride)
	{
		outputWidth = outputFrameWidth = TargetWidth;
		outputHeight = outputFrameHeight = TargetHeight;
		outputDirectBuffer = false;
	}
	else if (hasFormatChanged)
	{
		// dynamic output size switching
		outputWidth = avFrame->width;
		outputHeight = avFrame->height;
		outputFrameWidth = frameWidth;
		outputFrameHeight = frameHeight;

		VideoDescriptor->EncodingProperties->Width = outputFrameWidth;
		VideoDescriptor->EncodingProperties->Height = outputFrameHeight;

		MFVideoArea area;
		area.Area.cx = outputWidth;
		area.Area.cy = outputHeight;
		area.OffsetX.fract = 0;
		area.OffsetX.value = 0;
		area.OffsetY.fract = 0;
		area.OffsetY.value = 0;
		VideoDescriptor->EncodingProperties->Properties->Insert(MF_MT_MINIMUM_DISPLAY_APERTURE, ref new Array<uint8_t>((byte*)&area, sizeof(MFVideoArea)));
	}
}
