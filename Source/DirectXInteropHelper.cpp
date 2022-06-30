#include "pch.h"
#include "DirectXInteropHelper.h"
#include <windows.graphics.directx.direct3d11.interop.h>
#include <d3d11.h>
#include <mfidl.h>

using namespace Windows::Media::Core;
using namespace FFmpegInteropX;

IDirect3DSurface^ DirectXInteropHelper::GetSurface(IDXGISurface* source)
{
    return Windows::Graphics::DirectX::Direct3D11::CreateDirect3DSurface(source);
}

HRESULT DirectXInteropHelper::GetDXGISurface(IDirect3DSurface^ source, IDXGISurface** dxgiSurface)
{
    return Windows::Graphics::DirectX::Direct3D11::GetDXGIInterface(source, dxgiSurface);
}

HRESULT DirectXInteropHelper::GetDeviceManagerFromStreamSource(MediaStreamSource^ source, IMFDXGIDeviceManager** deviceManager)
{
    IMFDXGIDeviceManagerSource* surfaceManager = nullptr;
    auto unknownMss = reinterpret_cast<IUnknown*>(source);

    HRESULT hr = unknownMss->QueryInterface(&surfaceManager);

    if (SUCCEEDED(hr)) hr = surfaceManager->GetManager(deviceManager);

    SAFE_RELEASE(surfaceManager);
    return hr;
}

HRESULT DirectXInteropHelper::GetDeviceFromStreamSource(IMFDXGIDeviceManager* deviceManager, ID3D11Device** outDevice, ID3D11DeviceContext** outDeviceContext, ID3D11VideoDevice** outVideoDevice, HANDLE* outDeviceHandle)
{
    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* deviceContext = nullptr;
    ID3D11VideoDevice* videoDevice = nullptr;

    HRESULT hr = deviceManager->OpenDeviceHandle(outDeviceHandle);
    if (SUCCEEDED(hr)) hr = deviceManager->GetVideoService(*outDeviceHandle, IID_ID3D11Device, (void**)&device);
    if (SUCCEEDED(hr) && outDeviceContext) device->GetImmediateContext(&deviceContext);
    if (SUCCEEDED(hr) && outVideoDevice) hr = deviceManager->GetVideoService(*outDeviceHandle, IID_ID3D11VideoDevice, (void**)&videoDevice);

    if (!SUCCEEDED(hr))
    {
        SAFE_RELEASE(device);
        SAFE_RELEASE(deviceContext);
        SAFE_RELEASE(videoDevice);
    }
    else
    {
        *outDevice = device;
        if (outDeviceContext)
            *outDeviceContext = deviceContext;
        if (outVideoDevice)
            *outVideoDevice = videoDevice;
    }

    return hr;
}
