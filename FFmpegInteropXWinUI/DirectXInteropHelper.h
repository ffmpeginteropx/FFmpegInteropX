#pragma once
namespace FFmpegInteropX
{
    class DirectXInteropHelper
    {
    public:
        static winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DSurface GetSurface(IDXGISurface* source);
        static HRESULT GetDXGISurface2(winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DSurface source, IDXGISurface** dxgiSurface);
        static HRESULT GetDeviceFromStreamSource(IMFDXGIDeviceManager* deviceManager, ID3D11Device** outDevice, ID3D11DeviceContext** outDeviceContext, ID3D11VideoDevice** outVideoDevice, HANDLE* outDeviceHandle);
        static DXGI_ADAPTER_DESC GetDeviceDescription(ID3D11Device* device);
        static HRESULT GetDeviceManagerFromStreamSource(winrt::Windows::Media::Core::MediaStreamSource source, IMFDXGIDeviceManager** deviceManager);
    };
}
