#pragma once
#include "AssSsaRenderer.g.h"
#include <windows.graphics.directx.direct3d11.interop.h>

namespace winrt::AssSsaRenderElement::implementation
{
    struct AssSsaRenderer : AssSsaRendererT<AssSsaRenderer>
    {
        AssSsaRenderer() = default;

        AssSsaRenderer(winrt::Windows::UI::Xaml::Controls::SwapChainPanel const& swapChainPannel,
            winrt::FFmpegInteropX::FFmpegMediaSource const& mediaSource,
            winrt::Windows::Media::Playback::MediaPlaybackSession const& playbackSession)
        {
            this->swapChainPanel = swapChainPanel;
            this->playbackSession = playbackSession;
            timer = winrt::Windows::UI::Xaml::DispatcherTimer();
            timer.Tick(winrt::Windows::Foundation::EventHandler<winrt::Windows::Foundation::IInspectable>(this, &AssSsaRenderer::OnTick));
            this->mediaSource = mediaSource;
            SwapChainAllocResources(swapChainPannel);
        }

        void SetFrameUpdateInterval(winrt::Windows::Foundation::TimeSpan const& interval)
        {
            timer.Interval(interval);
        }

        void Play()
        {
            timer.Start();
        }

        void Pause()
        {
            timer.Stop();
        }

        void SetSelectedSubtitle(winrt::FFmpegInteropX::SubtitleStreamInfo const& subtitle)
        {
            this->subtitle = subtitle;
        }

    private:

        void OnTick(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& args)
        {
            winrt::com_ptr<ID3D11Texture2D> textureBuffer;
            swapChain->GetBuffer(0, IID_PPV_ARGS(textureBuffer.put()));

            winrt::com_ptr< IDXGISurface> dxgiSurface;
            textureBuffer.as(dxgiSurface);

            auto direct3DSurface = GetSurface(dxgiSurface);
            auto renderedResult = mediaSource.RenderSubtitlesToDirectXSurface(direct3DSurface, subtitle, playbackSession.Position());


            swapChain->Present(1, 0);
        }

        winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DSurface GetSurface(const winrt::com_ptr<IDXGISurface>& source)
        {
            winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DSurface result;
            winrt::com_ptr<::IInspectable> inspectableSurface;
            winrt::check_hresult(CreateDirect3D11SurfaceFromDXGISurface(source.get(), reinterpret_cast<::IInspectable**>(winrt::put_abi(inspectableSurface))));
            inspectableSurface.as(result);
            return result;
        }

        HRESULT SwapChainResizeBuffers(UINT bufferCount, UINT width, UINT heigh)
        {
            return swapChain->ResizeBuffers(bufferCount, width, heigh, DXGI_FORMAT_B8G8R8A8_UNORM, 0);
        }

        /// <summary>
        /// Used on first run or when device is lost
        /// </summary>
        /// <param name="swapChainPannel"></param>
        /// <returns></returns>
        HRESULT SwapChainAllocResources(winrt::Windows::UI::Xaml::Controls::SwapChainPanel const& swapChainPannel)
        {
            auto coreWindow = winrt::Windows::UI::Core::CoreWindow::GetForCurrentThread();

            UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#ifdef _DEBUG
            creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

            HRESULT hr = D3D11CreateDevice(
                nullptr,
                D3D_DRIVER_TYPE_HARDWARE,
                nullptr,
                creationFlags,
                nullptr,
                0,
                D3D11_SDK_VERSION,
                device.put(),
                nullptr,
                deviceContext.put()
            );

            device.as(dxgiDevice);
            hr = dxgiDevice->GetAdapter(dxgiAdapter.put());

            dxgiAdapter->GetParent(IID_PPV_ARGS(dxgiFactory.put()));

            DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};

            swapChainDesc.Width = 480; 
            swapChainDesc.Height = 480; 
            swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; 
            swapChainDesc.Stereo = false;
            swapChainDesc.SampleDesc.Count = 1; 
            swapChainDesc.SampleDesc.Quality = 0;
            swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            swapChainDesc.BufferCount = 2; 
            swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
            swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; 
            swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

            auto iunkownWindow = (coreWindow).as<IUnknown>();

            hr = dxgiFactory->CreateSwapChainForComposition(
                device.get(),
                &swapChainDesc,
                nullptr,
                swapChain.put()
            );

            com_ptr<ISwapChainPanelNative> swapChainNative;
            swapChainPanel.as(swapChainNative);


            swapChainNative->SetSwapChain(swapChain.get());

            return hr;
        }

        winrt::com_ptr<IDXGIFactory2> dxgiFactory;

        winrt::com_ptr<IDXGIDevice> dxgiDevice;
        winrt::com_ptr<IDXGIAdapter> dxgiAdapter;

        winrt::com_ptr<ID3D11Device> device;
        winrt::com_ptr<ID3D11DeviceContext> deviceContext;
        winrt::com_ptr<IDXGISwapChain1> swapChain;

        winrt::Windows::UI::Xaml::Controls::SwapChainPanel swapChainPanel;
        winrt::Windows::Media::Playback::MediaPlaybackSession playbackSession = { nullptr };

        winrt::Windows::UI::Xaml::DispatcherTimer timer;
        int backBufferIndex = 0;

        winrt::FFmpegInteropX::FFmpegMediaSource mediaSource = { nullptr };
        winrt::FFmpegInteropX::SubtitleStreamInfo subtitle = { nullptr };
    };
}
namespace winrt::AssSsaRenderElement::factory_implementation
{
    struct AssSsaRenderer : AssSsaRendererT<AssSsaRenderer, implementation::AssSsaRenderer>
    {
    };
}
