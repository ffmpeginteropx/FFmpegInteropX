#pragma once
#include <d3d10.h>
using namespace Windows::Graphics::DirectX::Direct3D11;

ref class DirectXInteropHelper
{
internal:
	static IDirect3DSurface^ GetSurface(IDXGISurface* source);
};

