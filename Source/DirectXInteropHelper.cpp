#include "pch.h"
#include "DirectXInteropHelper.h"
#include <Windows.Graphics.DirectX.Direct3D11.interop.h>

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

HRESULT DirectXInteropHelper::GetDeviceFromStreamSource(IMFDXGIDeviceManager* deviceManager, winrt::com_ptr<ID3D11Device>& outDevice, winrt::com_ptr<ID3D11DeviceContext>& outDeviceContext, winrt::com_ptr<ID3D11VideoDevice>& outVideoDevice, HANDLE* outDeviceHandle)
{
    winrt::com_ptr<ID3D11Device> device;
    winrt::com_ptr<ID3D11DeviceContext> deviceContext;
    winrt::com_ptr<ID3D11VideoDevice> videoDevice;

    HRESULT hr = deviceManager->OpenDeviceHandle(outDeviceHandle);
    if (SUCCEEDED(hr))
        hr = deviceManager->GetVideoService(*outDeviceHandle, IID_ID3D11Device, device.put_void());
    if (SUCCEEDED(hr))
        device->GetImmediateContext(deviceContext.put());
    if (SUCCEEDED(hr))
        hr = deviceManager->GetVideoService(*outDeviceHandle, IID_ID3D11VideoDevice, videoDevice.put_void());

    if (!SUCCEEDED(hr))
    {
        SAFE_RELEASE(device);
        SAFE_RELEASE(deviceContext);
        SAFE_RELEASE(videoDevice);
    }
    else
    {
        if (device)
            outDevice = device;
        if (deviceContext)
            outDeviceContext = deviceContext;
        if (videoDevice)
            outVideoDevice = videoDevice;
    }

    return hr;
}
