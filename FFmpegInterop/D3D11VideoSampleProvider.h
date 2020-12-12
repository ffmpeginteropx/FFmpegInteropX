#pragma once

#include "UncompressedVideoSampleProvider.h"
#include "TexturePool.h"

#include <d3d11.h>
#include <mfidl.h>
#include <DirectXInteropHelper.h>
#include <set>

extern "C"
{
#include <libavutil/hwcontext_d3d11va.h>
}

static AVPixelFormat get_format(struct AVCodecContext* s, const enum AVPixelFormat* fmt);

namespace FFmpegInterop
{
	using namespace Platform;

	ref class D3D11VideoSampleProvider : UncompressedVideoSampleProvider
	{
	internal:

		D3D11VideoSampleProvider(
			FFmpegReader^ reader,
			AVFormatContext* avFormatCtx,
			AVCodecContext* avCodecCtx,
			FFmpegInteropConfig^ config,
			int streamIndex,
			HardwareDecoderStatus hardwareDecoderStatus)
			: UncompressedVideoSampleProvider(reader, avFormatCtx, avCodecCtx, config, streamIndex, hardwareDecoderStatus)
		{
			decoder = DecoderEngine::FFmpegD3D11HardwareDecoder;
		}

		virtual HRESULT CreateBufferFromFrame(IBuffer^* pBuffer, IDirect3DSurface^* surface, AVFrame* avFrame, int64_t& framePts, int64_t& frameDuration) override
		{
			HRESULT hr = S_OK;

			if (avFrame->format == AV_PIX_FMT_D3D11)
			{
				if (!texturePool)
				{
					// init texture pool, fail if we did not get a device ptr
					if (device && deviceContext)
					{
						texturePool = ref new TexturePool(device, 5);
					}
					else
					{
						hr = E_FAIL;
					}
				}
				//copy texture data
				if (SUCCEEDED(hr))
				{
					////when one device decodes and the other renders (i.e laptops with nvidia optimus, amd enduro or desktops with multiple GPUs)
					//if (differentRenderDevice)
					//{
					//	AVFrame* cpuFrame = av_frame_alloc();
					//	av_hwframe_transfer_data(cpuFrame, avFrame, 0);
					//	av_frame_copy_props(cpuFrame, avFrame);

					//	hr = UncompressedVideoSampleProvider::CreateBufferFromFrame(pBuffer, surface, cpuFrame, framePts, frameDuration);
					//	av_frame_free(&cpuFrame);
					//	if (decoder != DecoderEngine::FFmpegSoftwareDecoder)
					//	{
					//		decoder = DecoderEngine::FFmpegSoftwareDecoder;
					//		VideoInfo->DecoderEngine = decoder;
					//	}
					//}
					//else
					{
						//cast the AVframe to texture 2D
						auto decodedTexture = reinterpret_cast<ID3D11Texture2D*>(avFrame->data[0]);
						ID3D11Texture2D* renderTexture = nullptr;
						//happy path:decoding and rendering on same GPU
						hr = texturePool->GetCopyTexture(decodedTexture, &renderTexture);
						deviceContext->CopySubresourceRegion(renderTexture, 0, 0, 0, 0, decodedTexture, (UINT)(unsigned long long)avFrame->data[1], NULL);
						deviceContext->Flush();


						//create a IDXGISurface from the shared texture
						IDXGISurface* finalSurface = NULL;
						if (SUCCEEDED(hr))
						{
							hr = renderTexture->QueryInterface(&finalSurface);
						}

						//get the IDirect3DSurface pointer
						if (SUCCEEDED(hr))
						{
							*surface = DirectXInteropHelper::GetSurface(finalSurface);
						}

						if (SUCCEEDED(hr))
						{
							CheckFrameSize(avFrame);
							ReadFrameProperties(avFrame, framePts);
						}

						SAFE_RELEASE(finalSurface);
						SAFE_RELEASE(renderTexture);
						if (decoder != DecoderEngine::FFmpegD3D11HardwareDecoder)
						{
							decoder = DecoderEngine::FFmpegD3D11HardwareDecoder;
							VideoInfo->DecoderEngine = decoder;
						}
					}
				}
			}
			else
			{
				hr = UncompressedVideoSampleProvider::CreateBufferFromFrame(pBuffer, surface, avFrame, framePts, frameDuration);

				if (decoder != DecoderEngine::FFmpegSoftwareDecoder)
				{
					decoder = DecoderEngine::FFmpegSoftwareDecoder;
					VideoInfo->DecoderEngine = decoder;
				}
			}

			return hr;
		}

		virtual HRESULT SetSampleProperties(MediaStreamSample^ sample) override
		{
			if (sample->Direct3D11Surface)
			{
				samples.insert(reinterpret_cast<IUnknown*>(sample));
				sample->Processed += ref new Windows::Foundation::TypedEventHandler<Windows::Media::Core::MediaStreamSample^, Platform::Object^>(this, &D3D11VideoSampleProvider::OnProcessed);
			}

			return UncompressedVideoSampleProvider::SetSampleProperties(sample);
		};

		void OnProcessed(Windows::Media::Core::MediaStreamSample^ sender, Platform::Object^ args)
		{
			auto unknown = reinterpret_cast<IUnknown*>(sender->Direct3D11Surface);
			IDXGISurface* surface = NULL;
			ID3D11Texture2D* texture = NULL;

			HRESULT hr = DirectXInteropHelper::GetDXGISurface(sender->Direct3D11Surface, &surface);

			if (SUCCEEDED(hr))
			{
				hr = surface->QueryInterface(&texture);
			}

			if (SUCCEEDED(hr))
			{
				texturePool->ReturnTexture(texture);
			}

			samples.erase(reinterpret_cast<IUnknown*>(sender));

			SAFE_RELEASE(surface);
			SAFE_RELEASE(texture);
		}

		virtual void SetHardwareDevice(ID3D11Device* device, ID3D11DeviceContext* context, AVBufferRef* avHardwareContext) override
		{
			if (!this->device)
			{
				device->AddRef();
				context->AddRef();
				this->device = device;
				this->deviceContext = context;

				if (m_pAvCodecCtx->hw_device_ctx->data != avHardwareContext->data)
				{
					av_buffer_unref(&m_pAvCodecCtx->hw_device_ctx);
					m_pAvCodecCtx->hw_device_ctx = av_buffer_ref(avHardwareContext);
				}
			}
			else
			{
				SAFE_RELEASE(this->device);
				SAFE_RELEASE(this->deviceContext);

				device->AddRef();
				context->AddRef();
				this->device = device;
				this->deviceContext = context;

				auto codec = m_pAvCodecCtx->codec;
				avcodec_free_context(&m_pAvCodecCtx);

				m_pAvCodecCtx = avcodec_alloc_context3(codec);
				m_pAvCodecCtx->get_format = &get_format;

				// initialize the stream parameters with demuxer information
				if (avcodec_parameters_to_context(m_pAvCodecCtx, m_pAvStream->codecpar) < 0)
				{
					//hr = E_FAIL;
				}

				m_pAvCodecCtx->hw_device_ctx = av_buffer_ref(avHardwareContext);

				if (avcodec_open2(m_pAvCodecCtx, codec, NULL) < 0)
				{
					//hr = E_FAIL;
				}

				frameProvider->UpdateCodecContext(m_pAvCodecCtx);
			}
		}

		static HRESULT InitializeHardwareDeviceContext(MediaStreamSource^ sender, AVBufferRef* avHardwareContext, ID3D11Device** outDevice, ID3D11DeviceContext** outDeviceContext)
		{
			ID3D11Device* device = nullptr;
			ID3D11DeviceContext* deviceContext = nullptr;
			ID3D11VideoDevice* videoDevice = nullptr;
			ID3D11VideoContext* videoContext = nullptr;
			HRESULT hr = DirectXInteropHelper::GetDeviceFromStreamSource(sender, &device, &deviceContext, &videoDevice);
			if (SUCCEEDED(hr)) hr = deviceContext->QueryInterface(&videoContext);

			auto dataBuffer = (AVHWDeviceContext*)avHardwareContext->data;
			auto internalDirectXHwContext = (AVD3D11VADeviceContext*)dataBuffer->hwctx;

			if (SUCCEEDED(hr))
			{
				// give ownership to FFmpeg

				internalDirectXHwContext->device = device;
				internalDirectXHwContext->device_context = deviceContext;
				internalDirectXHwContext->video_device = videoDevice;
				internalDirectXHwContext->video_context = videoContext;
			}
			else
			{
				// release
				SAFE_RELEASE(device);
				SAFE_RELEASE(deviceContext);
				SAFE_RELEASE(videoDevice);
				SAFE_RELEASE(videoContext);
			}

			if (SUCCEEDED(hr))
			{
				// multithread interface seems to be optional
				ID3D10Multithread* multithread;
				device->QueryInterface(&multithread);
				if (multithread)
				{
					multithread->SetMultithreadProtected(TRUE);
					multithread->Release();
				}
			}

			if (SUCCEEDED(hr))
			{
				auto err = av_hwdevice_ctx_init(avHardwareContext);
				if (err)
				{
					hr = E_FAIL;
				}
			}

			if (SUCCEEDED(hr))
			{
				// addref and hand out pointers
				device->AddRef();
				deviceContext->AddRef();
				*outDevice = device;
				*outDeviceContext = deviceContext;
			}

			return hr;
		}

		TexturePool^ texturePool;
		std::set<IUnknown*> samples;

	};
}

