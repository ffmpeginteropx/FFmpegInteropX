#include "pch.h"
#include "DirectXInteropHelper.h"
#include <windows.graphics.directx.direct3d11.interop.h>

IDirect3DSurface^ DirectXInteropHelper::GetSurface(IDXGISurface* source)
{
	return Windows::Graphics::DirectX::Direct3D11::CreateDirect3DSurface(source);
}