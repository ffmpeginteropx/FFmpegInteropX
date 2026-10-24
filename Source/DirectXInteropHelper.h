#pragma once
using namespace winrt::Windows::Graphics::DirectX::Direct3D11;

class DirectXInteropHelper
{
public:
    static IDirect3DSurface GetSurface(const winrt::com_ptr<IDXGISurface>& source);
    static HRESULT GetDXGISurface(const IDirect3DSurface& source, winrt::com_ptr<IDXGISurface>& dxgiSurface);
    static HRESULT GetDeviceFromStreamSource(const winrt::com_ptr<IMFDXGIDeviceManager>& deviceManager, winrt::com_ptr<ID3D11Device>& outDevice, winrt::com_ptr<ID3D11DeviceContext>& outDeviceContext, winrt::com_ptr<ID3D11VideoDevice>& outVideoDevice, HANDLE* outDeviceHandle);
    static HRESULT GetDeviceManagerFromStreamSource(const winrt::Windows::Media::Core::MediaStreamSource& source, winrt::com_ptr<IMFDXGIDeviceManager>& deviceManager);

    static HRESULT CreateTextureFromByteArray(
        ID3D11Device* device,
        const void* byteArray,
        UINT byteArraySize,
        UINT width,
        UINT height,
        DXGI_FORMAT format,
        ID3D11Texture2D** outTexture)
    {
        D3D11_TEXTURE2D_DESC textureDesc = {};

        textureDesc.Width = width;
        textureDesc.Height = height;
        textureDesc.MipLevels = 1;
        textureDesc.ArraySize = 1;
        textureDesc.Format = format;
        textureDesc.Usage = D3D11_USAGE_DEFAULT;
        textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        textureDesc.CPUAccessFlags = 0;
        textureDesc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;
        textureDesc.SampleDesc.Count = 1;

        D3D11_SUBRESOURCE_DATA initData = {};
        initData.pSysMem = byteArray;
        initData.SysMemPitch = width * 4;
        initData.SysMemSlicePitch = 0;

        HRESULT hr = device->CreateTexture2D(&textureDesc, &initData, outTexture);
        if (FAILED(hr)) {
            return hr;
        }
    }

    static winrt::com_ptr<ID3D11ShaderResourceView> CreateSRVFromTexture(winrt::com_ptr<ID3D11Device> device, winrt::com_ptr<ID3D11Texture2D> texture) {
        winrt::com_ptr<ID3D11ShaderResourceView> srv;
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = DXGI_FORMAT_R8_UNORM;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;

        HRESULT hr = device->CreateShaderResourceView(texture.get(), &srvDesc, srv.put());
        if (FAILED(hr)) {
            winrt::throw_hresult(winrt::hresult{ hr });
        }
        return srv;
    }

    static winrt::com_ptr<ID3D11SamplerState> CreateSampler(ID3D11Device* device)
    {
        D3D11_SAMPLER_DESC samplerDesc = {};
        samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; // Linear filtering
        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;    // Wrap texture addressing
        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.MipLODBias = 0.0f;
        samplerDesc.MaxAnisotropy = 1;
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
        samplerDesc.BorderColor[0] = 0.0f;
        samplerDesc.BorderColor[1] = 0.0f;
        samplerDesc.BorderColor[2] = 0.0f;
        samplerDesc.BorderColor[3] = 0.0f;
        samplerDesc.MinLOD = 0.0f;
        samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

        winrt::com_ptr<ID3D11SamplerState> samplerState;
        HRESULT hr = device->CreateSamplerState(&samplerDesc, samplerState.put());
        if (FAILED(hr)) {
            winrt::throw_hresult(winrt::hresult{ hr });
        }

        return samplerState;
    }
};
