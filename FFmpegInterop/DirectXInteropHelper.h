#pragma once
#include <d3d11.h>
#include <mfidl.h>
using namespace Windows::Graphics::DirectX::Direct3D11;
using namespace Windows::Media::Core;

ref class DirectXInteropHelper
{
internal:
	static IDirect3DSurface^ GetSurface(IDXGISurface* source);
	static HRESULT GetDXGISurface(IDirect3DSurface^ source, IDXGISurface** dxgiSurface);
	static HRESULT GetDeviceFromStreamSource(IMFDXGIDeviceManager* deviceManager, ID3D11Device** outDevice, ID3D11DeviceContext** outDeviceContext, ID3D11VideoDevice** outVideoDevice);
	static DXGI_ADAPTER_DESC GetDeviceDescription(ID3D11Device* device);
	static HRESULT GetDeviceManagerFromStreamSource(MediaStreamSource^ source, IMFDXGIDeviceManager** deviceManager);
};

