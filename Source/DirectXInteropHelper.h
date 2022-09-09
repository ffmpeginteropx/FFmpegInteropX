#pragma once
using namespace winrt::Windows::Graphics::DirectX::Direct3D11;

class DirectXInteropHelper
{
public:
    static IDirect3DSurface GetSurface(IDXGISurface* source);
    static HRESULT GetDXGISurface(IDirect3DSurface source, IDXGISurface** dxgiSurface);
    static HRESULT GetDeviceFromStreamSource(IMFDXGIDeviceManager* deviceManager, winrt::com_ptr<ID3D11Device> &outDevice, winrt::com_ptr<ID3D11DeviceContext>& outDeviceContext, winrt::com_ptr<ID3D11VideoDevice>& outVideoDevice, HANDLE* outDeviceHandle);
    static HRESULT GetDeviceManagerFromStreamSource(winrt::Windows::Media::Core::MediaStreamSource source, IMFDXGIDeviceManager** deviceManager);
};
