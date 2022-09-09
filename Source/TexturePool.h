#pragma once
#include <d3d11.h>
#include "DirectXInteropHelper.h"

class TexturePool
{
public:

    TexturePool(winrt::com_ptr<ID3D11Device> device, int initialPoolSize)
    {
        this->device = device;
        this->initialPoolSize = initialPoolSize;
    }

public:
    ~TexturePool()
    {
        Clear();
        SAFE_RELEASE(device);
    }

public:

    void Clear()
    {
        for (auto& texture : pool)
        {
            texture->Release();
        }
        pool.clear();
        ZeroMemory(&desc_shared, sizeof(desc_shared));
    }

    HRESULT Initialize(ID3D11Texture2D* sourceTexture)
    {
        if (pool.size() > 0)
        {
            Clear();
        }

        D3D11_TEXTURE2D_DESC desc;
        sourceTexture->GetDesc(&desc);

        //create a new texture description, with shared flag
        ZeroMemory(&desc_shared, sizeof(desc_shared));
        desc_shared.Width = desc.Width;
        desc_shared.Height = desc.Height;
        desc_shared.MipLevels = desc.MipLevels;
        desc_shared.ArraySize = 1;
        desc_shared.Format = desc.Format;
        desc_shared.SampleDesc.Count = desc.SampleDesc.Count;
        desc_shared.SampleDesc.Quality = desc.SampleDesc.Quality;
        desc_shared.Usage = D3D11_USAGE_DEFAULT;
        desc_shared.CPUAccessFlags = 0;
        desc_shared.MiscFlags = 0;
        desc_shared.BindFlags = desc.BindFlags;

        // pre-allocate pool
        ID3D11Texture2D* copy_tex;
        HRESULT hr = S_OK;
        for (int i = 0; i < initialPoolSize; i++)
        {
            hr = device->CreateTexture2D(&desc_shared, NULL, &copy_tex);

            if (SUCCEEDED(hr))
            {
                pool.push_back(WrapTexturePointer(copy_tex));
            }
            else
            {
                break;
            }
        }
        return hr;
    }

    HRESULT GetCopyTexture(ID3D11Texture2D* sourceTexture, ID3D11Texture2D** result)
    {
        HRESULT hr = S_OK;

        D3D11_TEXTURE2D_DESC desc;
        sourceTexture->GetDesc(&desc);

        if (desc.Width != desc_shared.Width || desc.Height != desc_shared.Height || desc.Format != desc_shared.Format)
        {
            hr = Initialize(sourceTexture);
        }

        if (SUCCEEDED(hr))
        {
            if (pool.size() > 0)
            {
                //use pool texture, if available
                pool.back().copy_to(result);
                pool.pop_back();
            }
            else
            {
                //otherwise create a new texture
                hr = device->CreateTexture2D(&desc_shared, NULL, result);
            }
        }

        return hr;
    }

    void ReturnTexture(ID3D11Texture2D* texture)
    {
        D3D11_TEXTURE2D_DESC desc;
        texture->GetDesc(&desc);

        if (desc.Width == desc_shared.Width && desc.Height == desc_shared.Height && desc.Format == desc_shared.Format)
        {
            pool.push_back(WrapTexturePointer(texture));
        }
    }

private:

    winrt::com_ptr<ID3D11Texture2D> WrapTexturePointer(ID3D11Texture2D* texture)
    {
        winrt::com_ptr<ID3D11Texture2D> ptr;
        ptr.copy_from(texture);
        return ptr;
    }

    winrt::com_ptr<ID3D11Device> device;
    std::vector<winrt::com_ptr<ID3D11Texture2D>> pool;
    D3D11_TEXTURE2D_DESC desc_shared{};
    int initialPoolSize = 0;
};

