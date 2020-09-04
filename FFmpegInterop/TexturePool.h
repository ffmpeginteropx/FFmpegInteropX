#pragma once
#include <d3d11.h>
#include <DirectXInteropHelper.h>

namespace FFmpegInterop
{
	using namespace Platform;

	ref class TexturePool
	{
	internal:

		TexturePool(ID3D11Device* device, int initialPoolSize)
		{
			this->device = device;
			this->initialPoolSize = initialPoolSize;
		}

	private:
		~TexturePool()
		{
			Clear();
		}

	internal:

		void Clear()
		{
			for each (auto texture in all)
			{
				texture->Release();
			}
			pool.clear();
			all.clear();
		}

		void Initialize(ID3D11Texture2D* sourceTexture)
		{
			if (all.size() > 0)
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
			HRESULT hr;
			for (int i = 0; i < initialPoolSize; i++)
			{
				hr = device->CreateTexture2D(&desc_shared, NULL, &copy_tex);

				if (SUCCEEDED(hr))
				{
					pool.push_back(copy_tex);
					all.push_back(copy_tex);
				}
				else
				{
					break;
				}
			}
		}

		ID3D11Texture2D* GetCopyTexture(ID3D11Texture2D* sourceTexture)
		{
			D3D11_TEXTURE2D_DESC desc;
			sourceTexture->GetDesc(&desc);

			if (desc.Width != desc_shared.Width || desc.Height != desc_shared.Height || desc.Format != desc_shared.Format)
			{
				Initialize(sourceTexture);
			}

			if (pool.size() > 0)
			{
				//use pool texture, if available
				auto result = pool.back();
				pool.pop_back();
				result->AddRef();
				return result;
			}
			else
			{
				//otherwise create a new texture
				ID3D11Texture2D* copy_tex;
				HRESULT hr = device->CreateTexture2D(&desc_shared, NULL, &copy_tex);
				if (SUCCEEDED(hr))
				{
					all.push_back(copy_tex);
					copy_tex->AddRef();
					return copy_tex;
				}

				return nullptr;
			}
		}

		void ReturnTexture(ID3D11Texture2D* texture)
		{
			D3D11_TEXTURE2D_DESC desc;
			texture->GetDesc(&desc);

			if (desc.Width == desc_shared.Width && desc.Height == desc_shared.Height && desc.Format == desc_shared.Format)
			{
				pool.push_back(texture);
			}
			else
			{
				// free texture if it cannot be re-used in pool
				texture->Release();
			}
		}

	private:
		ID3D11Device* device;
		std::vector<ID3D11Texture2D*> pool;
		std::vector<ID3D11Texture2D*> all;
		D3D11_TEXTURE2D_DESC desc_shared;
		int initialPoolSize;
	};

}