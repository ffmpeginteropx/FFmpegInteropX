#include "pch.h"
#include "DirectXInteropHelper.h"
extern "C" {
#include <Windows.Graphics.DirectX.Direct3D11.interop.h>
}
using namespace FFmpegInteropX;
//TODO: review this file later


winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DSurface DirectXInteropHelper::GetSurface(IDXGISurface* source)
{
	winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DSurface result;
	winrt::com_ptr<::IInspectable> inspectableSurface;
	if (winrt::check_hresult(CreateDirect3D11SurfaceFromDXGISurface(source, reinterpret_cast<::IInspectable**>(winrt::put_abi(inspectableSurface)))))
	{
		inspectableSurface.as(result);
		return result;
	}
	
	return nullptr;
}

HRESULT DirectXInteropHelper::GetDXGISurface2(winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DSurface source, IDXGISurface** dxgiSurface)
{
	using IUnknown = ::IUnknown;

	//TODO: this most likely doesn't work, but I left it this way to allow the project to build.
	winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice d3dDevice /* = ... */;
	winrt::com_ptr<::Windows::Graphics::DirectX::Direct3D11::IDirect3DDxgiInterfaceAccess> dxgiInterfaceAccess{
		d3dDevice.as<::Windows::Graphics::DirectX::Direct3D11::IDirect3DDxgiInterfaceAccess>()
	};
	winrt::com_ptr<::IDXGISurface> nativeSurface;
	winrt::check_hresult(dxgiInterfaceAccess->GetInterface(
		__uuidof(nativeSurface),
		nativeSurface.put_void()));

	*dxgiSurface = nativeSurface.get();
	return S_OK;
}

HRESULT DirectXInteropHelper::GetDeviceManagerFromStreamSource(winrt::Windows::Media::Core::MediaStreamSource source, IMFDXGIDeviceManager** deviceManager)
{
	//com_ptr<IMFDXGIDeviceManagerSource> surfaceManager;
	//com_ptr<IUnknown> unknownMss;
	//source.as(unknownMss);

	//HRESULT hr = unknownMss.get()->QueryInterface(surfaceManager.put_void());

	//if (SUCCEEDED(hr)) hr = surfaceManager->GetManager(deviceManager);

	////SAFE_RELEASE(surfaceManager);
	//return hr;
	return E_NOTIMPL;
}

HRESULT DirectXInteropHelper::GetDeviceFromStreamSource(IMFDXGIDeviceManager* deviceManager, ID3D11Device** outDevice, ID3D11DeviceContext** outDeviceContext, ID3D11VideoDevice** outVideoDevice, HANDLE* outDeviceHandle)
{
	//com_ptr<ID3D11Device> device;
	//com_ptr<ID3D11DeviceContext> deviceContext;
	//com_ptr<ID3D11VideoDevice> videoDevice;

	//HRESULT hr = deviceManager->OpenDeviceHandle(outDeviceHandle);
	//if (SUCCEEDED(hr)) hr = deviceManager->GetVideoService(*outDeviceHandle, IID_ID3D11Device, (void**)&device);
	//if (SUCCEEDED(hr) && outDeviceContext) device->GetImmediateContext(deviceContext.put());
	//if (SUCCEEDED(hr) && outVideoDevice) hr = deviceManager->GetVideoService(*outDeviceHandle, IID_ID3D11VideoDevice, (void**)&videoDevice);

	//if (!SUCCEEDED(hr))
	//{
	//	/*SAFE_RELEASE(device);
	//	SAFE_RELEASE(deviceContext);
	//	SAFE_RELEASE(videoDevice);*/
	//}
	//else
	//{
	//	*outDevice = device.get();
	//	if (outDeviceContext)
	//		*outDeviceContext = deviceContext.get();
	//	if (outVideoDevice)
	//		*outVideoDevice = videoDevice.get();
	//}

	return E_NOTIMPL;

}
