#pragma once
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.h>
#include <winrt/Microsoft.Graphics.Canvas.UI.Xaml.h>
#include <winrt/Windows.Graphics.Imaging.h>
#include <winrt/Windows.Media.Playback.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Microsoft.Graphics.Canvas.h> //This defines the C++/WinRT interfaces for the Win2D Windows Runtime Components
#include <Microsoft.Graphics.Canvas.native.h> //This is for interop
#include <winrt/Windows.UI.h>
#include <Windows.ui.xaml.media.dxinterop.h>
#include <d2d1.h>
#include <d2d1_2.h>
#include <winrt/Windows.Graphics.DirectX.Direct3D11.h>
#include <Microsoft.Graphics.Canvas.h>

namespace winrt::AssSsaRenderElement::implementation
{
    namespace abi {
        using namespace ABI::Microsoft::Graphics::Canvas;
    }

    using namespace winrt::Microsoft::Graphics;
    using namespace winrt::Microsoft::Graphics::Canvas;
    using namespace winrt::Microsoft::Graphics::Canvas::UI::Xaml;
    using namespace winrt::Windows::Graphics::Imaging;

    using namespace winrt::Windows::Foundation::Collections;
    using namespace winrt::Windows::Media::Effects;
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Windows::Media::MediaProperties;

    using namespace winrt::Microsoft::Graphics;
    using namespace winrt::Microsoft::Graphics::Canvas;
    using namespace winrt::Microsoft::Graphics::Canvas::Effects;

    using namespace winrt::Windows::Foundation;
    using namespace winrt::Windows::Media::Core;

    static void SwapChainAllocResources(const winrt::Windows::UI::Xaml::Controls::SwapChainPanel& swapChainPannel,
        float width,
        float height,
        float dpi, winrt::Windows::Graphics::DirectX::DirectXPixelFormat const& pixelFormat,
        int bufferCount,
        CanvasSwapChain& canvasSwapChain)
    {
        canvasSwapChain = CanvasSwapChain(CanvasDevice::GetSharedDevice(), width, height, dpi, pixelFormat, bufferCount, CanvasAlphaMode::Premultiplied);
        com_ptr<abi::ICanvasResourceWrapperNative> nativeDeviceWrapper = canvasSwapChain.as<abi::ICanvasResourceWrapperNative>();
        com_ptr<IDXGISwapChain1> pNativeSwapChain{ nullptr };
        check_hresult(nativeDeviceWrapper->GetNativeResource(nullptr, 0.0f, guid_of<IDXGISwapChain1>(), pNativeSwapChain.put_void()));

        auto nativeSwapChainPanel = swapChainPannel.as< ISwapChainPanelNative>();
        nativeSwapChainPanel->SetSwapChain(pNativeSwapChain.get());
    }

    static CanvasBitmap DXGISurface2CanvasBitmap(CanvasDevice sharedDevice, IDXGISurface* dxgiSurface)
    {
        //First we need to get an ID2D1Device1 pointer from the shared CanvasDevice
        com_ptr<abi::ICanvasResourceWrapperNative> nativeDeviceWrapper = sharedDevice.as<abi::ICanvasResourceWrapperNative>();
        com_ptr<ID2D1Device1> pDevice{ nullptr };
        check_hresult(nativeDeviceWrapper->GetNativeResource(nullptr, 0.0f, guid_of<ID2D1Device1>(), pDevice.put_void()));

        com_ptr<ID2D1DeviceContext1> pContext{ nullptr };
        check_hresult(pDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, pContext.put()));

        com_ptr<ID2D1Bitmap1> bitmap;
        D2D1_BITMAP_PROPERTIES1 bitmapProperties = {};

        DXGI_SURFACE_DESC dxgiDesc;
        dxgiSurface->GetDesc(&dxgiDesc);

        auto bla = bitmap.put();

        pContext->CreateBitmapFromDxgiSurface(dxgiSurface, nullptr, bitmap.put());


        com_ptr<::IInspectable> pInspectable{ nullptr };
        auto factory = winrt::get_activation_factory<CanvasDevice, abi::ICanvasFactoryNative>(); //abi::ICanvasFactoryNative is the activation factory for the CanvasDevice class
        check_hresult(factory->GetOrCreate(sharedDevice.as<abi::ICanvasDevice>().get(), bitmap.as<::IUnknown>().get(), 0.0f, pInspectable.put())); //Note abi::ICanvasDevice is defined in the header Microsoft.Graphics.Canvas.h
        CanvasBitmap cvb = pInspectable.as<CanvasBitmap>();
        return cvb;
    }

    HRESULT GetDXGISurface(const winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DSurface& source, winrt::com_ptr<IDXGISurface>& dxgiSurface)
    {
        auto dxgiInterfaceAccess = source.as<::Windows::Graphics::DirectX::Direct3D11::IDirect3DDxgiInterfaceAccess>();

        winrt::com_ptr<IDXGISurface> nativeSurface;
        auto hr = dxgiInterfaceAccess->GetInterface(
            __uuidof(nativeSurface),
            dxgiSurface.put_void());

        return hr;
    }
}
