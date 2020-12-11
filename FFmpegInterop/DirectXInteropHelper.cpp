#include "pch.h"
#include "DirectXInteropHelper.h"
#include <windows.graphics.directx.direct3d11.interop.h>
#include <d3d11.h>
#include <mfidl.h>
using namespace Windows::Media::Core;

IDirect3DSurface^ DirectXInteropHelper::GetSurface(IDXGISurface* source)
{
	return Windows::Graphics::DirectX::Direct3D11::CreateDirect3DSurface(source);
}

HRESULT DirectXInteropHelper::GetDXGISurface(IDirect3DSurface^ source, IDXGISurface** dxgiSurface)
{
	return Windows::Graphics::DirectX::Direct3D11::GetDXGIInterface(source, dxgiSurface);
}

HRESULT DirectXInteropHelper::GetDeviceFromStreamSource(MediaStreamSource^ source, ID3D11Device** outDevice, ID3D11DeviceContext** outDeviceContext, ID3D11VideoDevice** outVideoDevice)
{
	IMFDXGIDeviceManagerSource* surfaceManager = nullptr;
	IMFDXGIDeviceManager* deviceManager = nullptr;
	HANDLE deviceHandle = INVALID_HANDLE_VALUE;
	ID3D11Device* device = nullptr;
	ID3D11DeviceContext* deviceContext = nullptr;

	auto unknownMss = reinterpret_cast<IUnknown*>(source);

	HRESULT hr = unknownMss->QueryInterface(&surfaceManager);

	if (SUCCEEDED(hr)) hr = surfaceManager->GetManager(&deviceManager);
	if (SUCCEEDED(hr)) hr = deviceManager->OpenDeviceHandle(&deviceHandle);
	if (SUCCEEDED(hr)) hr = deviceManager->GetVideoService(deviceHandle, IID_ID3D11Device, (void**)&device);
	if (SUCCEEDED(hr) && outDeviceContext) device->GetImmediateContext(&deviceContext);
	if (SUCCEEDED(hr) && outVideoDevice) hr = deviceManager->GetVideoService(deviceHandle, IID_ID3D11VideoDevice, (void**)&outVideoDevice);

	SAFE_RELEASE(deviceManager);
	SAFE_RELEASE(surfaceManager);

	if (!SUCCEEDED(hr))
	{
		SAFE_RELEASE(device);
		SAFE_RELEASE(deviceContext);
		if (outVideoDevice)
			delete outVideoDevice;
	}

	*outDevice = device;
	if (outDeviceContext)
		*outDeviceContext = deviceContext;

	return hr;
}

DXGI_ADAPTER_DESC DirectXInteropHelper::GetDeviceDescription(ID3D11Device* device)
{
	IDXGIDevice* dxgiDevice = nullptr;
	IDXGIAdapter* adapter = nullptr;
	DXGI_ADAPTER_DESC returnValue;

	HRESULT hr = device->QueryInterface(&dxgiDevice);
	if (SUCCEEDED(hr)) hr = dxgiDevice->GetAdapter(&adapter);
	if (SUCCEEDED(hr)) adapter->GetDesc(&returnValue);

	SAFE_RELEASE(dxgiDevice);
	SAFE_RELEASE(adapter);

	return returnValue;
}
