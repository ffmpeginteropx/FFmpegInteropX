#include "pch.h"
#include "DirectXInteropHelper.h"
#include <Windows.Graphics.DirectX.Direct3D11.interop.h>

using namespace FFmpegInteropX;
using namespace winrt::Windows::Graphics::DirectX::Direct3D11;

IDirect3DSurface DirectXInteropHelper::GetSurface(IDXGISurface* source)
{
    IDirect3DSurface result;
    winrt::com_ptr<::IInspectable> inspectableSurface;
    winrt::check_hresult(CreateDirect3D11SurfaceFromDXGISurface(source, reinterpret_cast<::IInspectable**>(winrt::put_abi(inspectableSurface))));
    inspectableSurface.as(result);
    return result;
}

HRESULT DirectXInteropHelper::GetDXGISurface(IDirect3DSurface source, IDXGISurface** dxgiSurface)
{
    auto dxgiInterfaceAccess = source.as<::Windows::Graphics::DirectX::Direct3D11::IDirect3DDxgiInterfaceAccess>();

    winrt::com_ptr<IDXGISurface> nativeSurface;
    auto hr = dxgiInterfaceAccess->GetInterface(
        __uuidof(nativeSurface),
        nativeSurface.put_void());

    if (SUCCEEDED(hr))
    {
        *dxgiSurface = nativeSurface.detach();
    }
    return hr;

}

HRESULT DirectXInteropHelper::GetDeviceManagerFromStreamSource(winrt::Windows::Media::Core::MediaStreamSource source, IMFDXGIDeviceManager** deviceManager)
{
    winrt::com_ptr<IMFDXGIDeviceManagerSource> surfaceManager;
    source.as(surfaceManager);
    auto hr = surfaceManager->GetManager(deviceManager);
    return hr;
}

HRESULT DirectXInteropHelper::GetDeviceFromStreamSource(IMFDXGIDeviceManager* deviceManager, ID3D11Device** outDevice, ID3D11DeviceContext** outDeviceContext, ID3D11VideoDevice** outVideoDevice, HANDLE* outDeviceHandle)
{
    ID3D11Device* device = NULL;
    ID3D11DeviceContext* deviceContext = NULL;
    ID3D11VideoDevice* videoDevice = NULL;

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
        if (device)
            *outDevice = device;
        if (deviceContext)
            *outDeviceContext = deviceContext;
        if (videoDevice)
            *outVideoDevice = videoDevice;
    }

    return hr;
}
