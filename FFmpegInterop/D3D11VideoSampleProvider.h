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
				if (!renderDevice) renderDevice = device;
				if (!renderDeviceContext) renderDeviceContext = deviceContext;

				bool differentRenderDevice = false;
				auto desc1 = DirectXInteropHelper::GetDeviceDescription(device);
				ID3D11Device* newDevice;
				ID3D11DeviceContext* newContext;
				ID3D11VideoDevice* videoDevice;

				hr = DirectXInteropHelper::GetDeviceFromStreamSource(MediaStreamSourceInstance, &newDevice, &newContext, &videoDevice);
				if (SUCCEEDED(hr))
				{
					auto desc2 = DirectXInteropHelper::GetDeviceDescription(newDevice);
					if (desc1.DeviceId != desc2.DeviceId)
					{
						OutputDebugStringW(L"\n new device\n");
						differentRenderDevice = true;
						renderDevice = newDevice;
						renderDeviceContext = newContext;
					}
					else
					{
						SAFE_RELEASE(newDevice);
						SAFE_RELEASE(newContext);
						differentRenderDevice = false;
					}
				}
				if (!texturePool || differentRenderDevice)
				{
					// init texture pool, fail if we did not get a device ptr
					if (renderDevice && renderDeviceContext)
					{
						texturePool = ref new TexturePool(renderDevice, 1);
					}
					else
					{
						hr = E_FAIL;
					}
				}

				//cast the AVframe to texture 2D
				auto decodedTexture = reinterpret_cast<ID3D11Texture2D*>(avFrame->data[0]);
				ID3D11Texture2D* renderTexture = nullptr;



				//copy texture data
				if (SUCCEEDED(hr))
				{
					//when one device decodes and the other renders (i.e laptops with nvidia optimus, amd enduro or desktops with multiple GPUs)
					if (differentRenderDevice)
					{
						HANDLE copyHandle;
						IDXGIResource* dxgiResource;

						ID3D11Texture2D* renderTextureTemp = nullptr;
						D3D11_TEXTURE2D_DESC desc;
						decodedTexture->GetDesc(&desc);
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
						
						hr = renderDevice->CreateTexture2D(&desc_shared, NULL, &renderTextureTemp);
										
						renderTextureTemp->QueryInterface(&dxgiResource);
						dxgiResource->GetSharedHandle(&copyHandle);
													
						renderTexture = nullptr;

						hr = device->OpenSharedResource(copyHandle, IID_ID3D11Texture2D, (void**)&renderTexture);
						deviceContext->Flush();
						deviceContext->CopySubresourceRegion(renderTexture, 0, 0, 0, 0, decodedTexture, (UINT)(unsigned long long)avFrame->data[1], NULL);

						deviceContext->Flush();
						renderDeviceContext->Flush();
						SAFE_RELEASE(renderTextureTemp);
						SAFE_RELEASE(dxgiResource);
					}
					else
					{
						//happy path:decoding and rendering on same GPU
						hr = texturePool->GetCopyTexture(decodedTexture, &renderTexture);
						renderDeviceContext->CopySubresourceRegion(renderTexture, 0, 0, 0, 0, decodedTexture, (UINT)(unsigned long long)avFrame->data[1], NULL);
						renderDeviceContext->Flush();
					}
				}

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
					ReadFrameProperties(avFrame, framePts);
				}

				SAFE_RELEASE(finalSurface);
				SAFE_RELEASE(renderTexture);
				SAFE_RELEASE(newDevice);
				SAFE_RELEASE(newContext);
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

