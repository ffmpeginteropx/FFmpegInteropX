#pragma once
#include <stdio.h>
#include <crtdbg.h>
#include <d3dcommon.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <conio.h>
#include <iostream>
#include <fstream>
#include <mfapi.h>
#include <mfidl.h>
#include <dxva2api.h>

using namespace std;
using namespace Windows::Media::MediaProperties;

namespace FFmpegInterop
{
	ref class DirectXDecoderManager
	{
		IMFDXGIDeviceManagerSource* deviceSource = nullptr;;
		IMFDXGIDeviceManager* deviceManager;
		ID3D11Device* g_pDevice = nullptr;
		ID3D11DeviceContext* g_pContext;
		ID3D11VideoDevice* videoDecoderDevice;
		ID3D11VideoContext* videoDecoderContext;
		ID3D10Multithread* threadProtection;
		AVCodecContext* m_pAvCodecContext;
		HANDLE GPUHandle;


	internal:

		DirectXDecoderManager(IMFDXGIDeviceManagerSource* _deviceSource, AVCodecContext* ffmpegContext)
		{
			m_pAvCodecContext = ffmpegContext;
			deviceSource = _deviceSource;

			HRESULT hr = deviceSource->GetManager(&deviceManager);
			deviceManager->OpenDeviceHandle(&GPUHandle);

			void* unknownDeviceHandle;
			deviceManager->GetVideoService(GPUHandle, IID_ID3D11Device, &unknownDeviceHandle);

			g_pDevice = reinterpret_cast<ID3D11Device*>(unknownDeviceHandle);
			void* unknownVideoDevcieHandle;

			deviceManager->GetVideoService(GPUHandle, IID_ID3D11VideoDevice, &unknownVideoDevcieHandle);
			videoDecoderDevice = reinterpret_cast<ID3D11VideoDevice*>(unknownVideoDevcieHandle);

			g_pDevice->GetImmediateContext(&g_pContext);
			hr = g_pContext->QueryInterface(&videoDecoderContext);

			g_pDevice->QueryInterface(&threadProtection);
			threadProtection->SetMultithreadProtected(true);
		}

		void Test()
		{
			HRESULT hr = S_OK;
			auto decodersCount = videoDecoderDevice->GetVideoDecoderProfileCount();
			//{f7e34c9a - 42e8 - 4714 - b74b - cb29d72c35e5}
			//find the proper decoder for media stream descriptor
			GUID config;

			for (UINT i = 0; i < decodersCount; i++)
			{
				videoDecoderDevice->GetVideoDecoderProfile(i, &config);

				if (IsEqualGUID(D3D11_DECODER_PROFILE_HEVC_VLD_MAIN10, config))
				{
					break;
				}
			}

			BOOL isSupported;
			videoDecoderDevice->CheckVideoDecoderFormat(&config, DXGI_FORMAT_NV12, &isSupported);
			UINT profilesCount;

			D3D11_VIDEO_DECODER_DESC decoderDesc{
				 config,
				 m_pAvCodecContext->width,
				 m_pAvCodecContext->height,
				 DXGI_FORMAT_NV12,
			};

			hr = videoDecoderDevice->GetVideoDecoderConfigCount(&decoderDesc, &profilesCount);
			OutputDebugString(profilesCount.ToString()->Data());
			D3D11_VIDEO_DECODER_CONFIG videoDecoderConfig;
			for (UINT i = 0; i < profilesCount; i++)
			{
				videoDecoderDevice->GetVideoDecoderConfig(&decoderDesc, i, &videoDecoderConfig);
				if (videoDecoderConfig.guidConfigBitstreamEncryption == DXVA2_NoEncrypt
					&& videoDecoderConfig.guidConfigMBcontrolEncryption == DXVA2_NoEncrypt
					&& videoDecoderConfig.guidConfigResidDiffEncryption == DXVA2_NoEncrypt)
				{
					break;
				}
			}
			m_pAvCodecContext->


		}



		template <class T> void SafeRelease(T** ppT)
		{
			if (*ppT)
			{
				(*ppT)->Release();
				*ppT = NULL;
			}
		}

	};
}