#include "pch.h"
#include "DirectXInteropHelper.h"
#include <Windows.Graphics.DirectX.Direct3D11.interop.h>

using namespace winrt::Windows::Graphics::DirectX::Direct3D11;

IDirect3DSurface DirectXInteropHelper::GetSurface(const winrt::com_ptr<IDXGISurface>& source)
{
    IDirect3DSurface result;
    winrt::com_ptr<::IInspectable> inspectableSurface;
    winrt::check_hresult(CreateDirect3D11SurfaceFromDXGISurface(source.get(), reinterpret_cast<::IInspectable**>(winrt::put_abi(inspectableSurface))));
    inspectableSurface.as(result);
    return result;
}

HRESULT DirectXInteropHelper::GetDXGISurface(const IDirect3DSurface& source, winrt::com_ptr<IDXGISurface>& dxgiSurface)
{
    auto dxgiInterfaceAccess = source.as<::Windows::Graphics::DirectX::Direct3D11::IDirect3DDxgiInterfaceAccess>();

    winrt::com_ptr<IDXGISurface> nativeSurface;
    auto hr = dxgiInterfaceAccess->GetInterface(
        __uuidof(nativeSurface),
        dxgiSurface.put_void());

    return hr;
}

HRESULT DirectXInteropHelper::GetDeviceManagerFromStreamSource(const winrt::Windows::Media::Core::MediaStreamSource& source, winrt::com_ptr<IMFDXGIDeviceManager>& deviceManager)
{
    winrt::com_ptr<IMFDXGIDeviceManagerSource> surfaceManager;
    source.as(surfaceManager);
    auto hr = surfaceManager->GetManager(deviceManager.put());
    return hr;
}

HRESULT DirectXInteropHelper::GetD3D11DeviceFromStreamSource(
    const winrt::com_ptr<IMFDXGIDeviceManager>& deviceManager,
    winrt::com_ptr<ID3D11Device>& outDevice,
    winrt::com_ptr<ID3D11DeviceContext>& outDeviceContext,
    winrt::com_ptr<ID3D11VideoDevice>& outVideoDevice,
    HANDLE* outDeviceHandle)
{

    HRESULT hr = deviceManager->OpenDeviceHandle(outDeviceHandle);
    if (SUCCEEDED(hr))
        hr = deviceManager->GetVideoService(*outDeviceHandle, IID_ID3D11Device, outDevice.put_void());
    if (SUCCEEDED(hr))
        outDevice->GetImmediateContext(outDeviceContext.put());
    if (SUCCEEDED(hr))
        hr = deviceManager->GetVideoService(*outDeviceHandle, IID_ID3D11VideoDevice, outVideoDevice.put_void());

    if (!SUCCEEDED(hr))
    {
        outDevice = nullptr;
        outDeviceContext = nullptr;
        outVideoDevice = nullptr;
    }

    return hr;
}

HRESULT DirectXInteropHelper::GetD3D12DeviceFromStreamSource(
    const winrt::com_ptr<IMFDXGIDeviceManager>& deviceManager,
    winrt::com_ptr<ID3D12Device>& outDevice)
{
    HANDLE deviceHandle = nullptr;
    winrt::com_ptr<ID3D11Device> d3d11Device;
    HRESULT hr = deviceManager->OpenDeviceHandle(&deviceHandle);
    if (SUCCEEDED(hr))
    {
        hr = deviceManager->GetVideoService(deviceHandle, IID_ID3D11Device, d3d11Device.put_void());
    }

    winrt::com_ptr<IDXGIDevice> dxgiDevice;
    winrt::com_ptr<IDXGIAdapter> adapter;
    if (SUCCEEDED(hr))
    {
        dxgiDevice = d3d11Device.try_as<IDXGIDevice>();
        hr = dxgiDevice ? dxgiDevice->GetAdapter(adapter.put()) : E_NOINTERFACE;
    }

    if (SUCCEEDED(hr))
    {
        hr = D3D12CreateDevice(adapter.get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), outDevice.put_void());
    }

    if (deviceHandle)
    {
        deviceManager->CloseDeviceHandle(deviceHandle);
    }

    if (FAILED(hr))
    {
        outDevice = nullptr;
    }

    return hr;
}
