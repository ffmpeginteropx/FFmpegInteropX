#pragma once
using namespace winrt::Windows::Graphics::DirectX::Direct3D11;

class DirectXInteropHelper
{
public:
    static IDirect3DSurface GetSurface(const winrt::com_ptr<IDXGISurface>& source);
    static HRESULT GetDXGISurface(const IDirect3DSurface& source, winrt::com_ptr<IDXGISurface>& dxgiSurface);
    static HRESULT GetDeviceFromStreamSource(const winrt::com_ptr<IMFDXGIDeviceManager>& deviceManager, winrt::com_ptr<ID3D11Device> &outDevice, winrt::com_ptr<ID3D11DeviceContext>& outDeviceContext, winrt::com_ptr<ID3D11VideoDevice>& outVideoDevice, HANDLE* outDeviceHandle);
    static HRESULT GetDeviceManagerFromStreamSource(const winrt::Windows::Media::Core::MediaStreamSource& source, winrt::com_ptr<IMFDXGIDeviceManager>& deviceManager);
};
