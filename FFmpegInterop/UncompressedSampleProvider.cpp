//*****************************************************************************
//
//	Copyright 2016 Microsoft Corporation
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
#include "UncompressedSampleProvider.h"
#include <EventToken.h>
#include <d3d11.h>
#include <DirectXInteropHelper.h>

using namespace FFmpegInterop;


UncompressedSampleProvider::UncompressedSampleProvider(
	FFmpegReader^ reader,
	AVFormatContext* avFormatCtx,
	AVCodecContext* avCodecCtx,
	FFmpegInteropConfig^ config,
	int streamIndex,
	HardwareDecoderStatus hardwareDecoderStatus
) : MediaSampleProvider(reader, avFormatCtx, avCodecCtx, config, streamIndex, hardwareDecoderStatus)
{
	decoder = DecoderEngine::FFmpegSoftwareDecoder;
}

HRESULT UncompressedSampleProvider::CreateNextSampleBuffer(IBuffer^* pBuffer, int64_t& samplePts, int64_t& sampleDuration, IDirect3DSurface^* surface)
{
	HRESULT hr = S_OK;

	AVFrame* avFrame = av_frame_alloc();
	unsigned int errorCount = 0;

	if (!avFrame)
	{
		hr = E_OUTOFMEMORY;
	}

	while (SUCCEEDED(hr))
	{
		hr = GetFrameFromFFmpegDecoder(avFrame, samplePts, sampleDuration);

		if (hr == S_FALSE)
		{
			// end of stream reached
			break;
		}

		if (SUCCEEDED(hr))
		{
			if (m_pAvCodecCtx->hw_device_ctx)
			{
				//cast the AVframe to texture 2D
				auto nativeSurface = reinterpret_cast<ID3D11Texture2D*>(avFrame->data[0]);
				ID3D11Texture2D* frameData;
				D3D11_TEXTURE2D_DESC desc;

				ID3D11Device* ffmpegDevice;
				ID3D11DeviceContext* ffmpegDevcieContext;
				ID3D11DeviceContext* mssDevcieContext;
				//get the MSS device context
				GraphicsDevice->GetImmediateContext(&mssDevcieContext);
				//get the ffmpeg device pointer and its context
				nativeSurface->GetDevice(&ffmpegDevice);
				
				ffmpegDevice->GetImmediateContext(&ffmpegDevcieContext);
				//get the description of ffmpeg texture 2D
				nativeSurface->GetDesc(&desc);

				//create a new texture description, with shared flag
				D3D11_TEXTURE2D_DESC desc_shared;
				ZeroMemory(&desc_shared, sizeof(desc_shared));
				desc_shared.Width = desc.Width;
				desc_shared.Height = desc.Height;
				desc_shared.MipLevels = desc.MipLevels;
				desc_shared.ArraySize = 1;
				desc_shared.Format = desc.Format;
				desc_shared.SampleDesc.Count = desc.SampleDesc.Count;
				desc_shared.SampleDesc.Quality = desc.SampleDesc.Quality;
				desc_shared.Usage = D3D11_USAGE_DEFAULT;
				desc_shared.CPUAccessFlags = 0;
				desc_shared.MiscFlags = D3D11_RESOURCE_MISC_SHARED;
				desc_shared.BindFlags = desc.BindFlags;
				ID3D11Texture2D* copy_tex;
				//created a shared texture 2D, on the MSS device pointer
				hr = GraphicsDevice->CreateTexture2D(&desc_shared, NULL, &copy_tex);

				//ge a handle to that shared texture
				HANDLE sharedHandle;
				IDXGIResource* dxgiResource;
				copy_tex->QueryInterface(&dxgiResource);
				
				hr = dxgiResource->GetSharedHandle(&sharedHandle);
				SAFE_RELEASE(dxgiResource);

				ID3D11Texture2D* sharedTexture;
				//open the shared handle on the ffmpeg device
				HRESULT hr = ffmpegDevice->OpenSharedResource(sharedHandle, IID_ID3D11Texture2D, (void**)&sharedTexture);
				//copy the last array of the ffmpeg texture to the shared texture
				ffmpegDevcieContext->CopySubresourceRegion(sharedTexture, 0, 0, 0, 0,  nativeSurface, (UINT)avFrame->data[1], NULL);
				//flush the contextes to ensure the GPU data is updated
				ffmpegDevcieContext->Flush();
				mssDevcieContext->Flush();
				//create a IDXGISurface from the shared texture
				IDXGISurface* finalSurface = NULL;
				sharedTexture->QueryInterface(&finalSurface);
				//get the IDirect3DSurface pointer
				*surface = DirectXInteropHelper::GetSurface(finalSurface);
				SAFE_RELEASE(ffmpegDevice);
				SAFE_RELEASE(ffmpegDevcieContext);
				SAFE_RELEASE(copy_tex);
				SAFE_RELEASE(sharedTexture);	
			}
			else {
				hr = CreateBufferFromFrame(pBuffer, avFrame, samplePts, sampleDuration);
			}
			if (SUCCEEDED(hr))
			{
				// sample created. update m_nextFramePts in case pts or duration have changed
				nextFramePts = samplePts + sampleDuration;
				break;
			}
		}

		if (!SUCCEEDED(hr) && errorCount++ < m_config->SkipErrors)
		{
			// unref any buffers in old frame
			av_frame_unref(avFrame);

			// try a few more times
			m_isDiscontinuous = true;
			hr = S_OK;
		}
	}

	if (avFrame)
	{
		av_frame_free(&avFrame);
	}

	return hr;
}

HRESULT UncompressedSampleProvider::GetFrameFromFFmpegDecoder(AVFrame* avFrame, int64_t& framePts, int64_t& frameDuration)
{
	HRESULT hr = S_OK;

	while (SUCCEEDED(hr))
	{
		HRESULT decodeFrame;
		// Try to get a frame from the decoder.
		if (frameProvider)
		{
			decodeFrame = frameProvider->GetFrameFromCodec(avFrame);
		}
		if (hwFrameProvider)
		{
			//AVFrame* hwFrame = av_frame_alloc();
			decodeFrame = hwFrameProvider->GetHWFrameFromCodec(avFrame, avFrame);
			/*av_frame_free(&hwFrame);*/

		}
		if (decodeFrame == AVERROR(EAGAIN))
		{
			// The decoder doesn't have enough data to produce a frame,
			// we need to feed a new packet.
			hr = FeedPacketToDecoder();
		}
		else if (decodeFrame == AVERROR_EOF)
		{
			DebugMessage(L"End of stream reached. No more samples in decoder.\n");
			hr = S_FALSE;
			break;
		}
		else if (decodeFrame < 0)
		{
			DebugMessage(L"Failed to get a frame from the decoder\n");
			hr = E_FAIL;
			break;
		}
		else
		{
			// Update the timestamp
			if (avFrame->pts != AV_NOPTS_VALUE)
			{
				framePts = avFrame->pts;
			}
			else
			{
				framePts = nextFramePts;
			}

			frameDuration = avFrame->pkt_duration;
			nextFramePts = framePts + frameDuration;

			hr = S_OK;
			break;
		}
	}

	return hr;
}

HRESULT UncompressedSampleProvider::FeedPacketToDecoder()
{
	HRESULT hr = S_OK;

	AVPacket* avPacket = NULL;
	LONGLONG pts = 0;
	LONGLONG dur = 0;

	hr = GetNextPacket(&avPacket, pts, dur);
	if (hr == S_FALSE)
	{
		// End of stream reached. Feed NULL packet to decoder to enter draining mode.
		DebugMessage(L"End of stream reached. Enter draining mode.\n");
		int sendPacketResult = avcodec_send_packet(m_pAvCodecCtx, NULL);
		if (sendPacketResult < 0)
		{
			hr = E_FAIL;
			DebugMessage(L"Decoder failed to enter draining mode.\n");
		}
		else
		{
			hr = S_OK;
		}
	}
	else if (SUCCEEDED(hr))
	{
		// Feed packet to decoder.
		int sendPacketResult = avcodec_send_packet(m_pAvCodecCtx, avPacket);
		if (sendPacketResult == AVERROR(EAGAIN))
		{
			// The decoder should have been drained and always ready to access input
			_ASSERT(FALSE);
			hr = E_UNEXPECTED;
		}
		else if (sendPacketResult < 0)
		{
			// We failed to send the packet
			hr = E_FAIL;
			DebugMessage(L"Decoder failed on the sample.\n");
		}

		// store first packet pts as nextFramePts, in case frames do not carry correct pts values
		if (SUCCEEDED(hr) && !hasNextFramePts)
		{
			nextFramePts = pts;
			hasNextFramePts = true;
		}
	}

	av_packet_free(&avPacket);

	return hr;
}

void UncompressedSampleProvider::Flush()
{
	MediaSampleProvider::Flush();

	// after seek we need to get first packet pts again
	hasNextFramePts = false;
}