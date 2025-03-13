#pragma once
#include "AssSsaRenderer.g.h"
#include <windows.graphics.directx.direct3d11.interop.h>
#include "winrt/Microsoft.Graphics.Canvas.h"
#include "Win2DInteropHelpers.h"

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

            SwapChainAllocResources(swapChainPannel,
                swapChainPanel.ActualWidth(),
                swapChainPanel.ActualHeight(),
                96,
                winrt::Windows::Graphics::DirectX::DirectXPixelFormat::B8G8R8A8UIntNormalized,
                2,
                canvasSwapChain);
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

        }

        void RenderSubtitleFrame(winrt::Windows::Media::Playback::MediaPlayer const& player,
            uint32_t width,
            uint32_t height,
            uint32_t dpi,
            winrt::Windows::Graphics::DirectX::DirectXPixelFormat const& pixelFormat)
        {
            try {
                auto canvasDevice = CanvasDevice::GetSharedDevice();
                if (canvasSwapChain.Device().IsDeviceLost())
                {
                    SwapChainAllocResources(this->swapChainPanel, width, height, dpi, pixelFormat, 2, canvasSwapChain);
                }

                if (canvasSwapChain.Format() != pixelFormat || (uint32_t)canvasSwapChain.Size().Width != (uint32_t)width || (uint32_t)canvasSwapChain.Size().Height != (uint32_t)height || (uint32_t)canvasSwapChain.Dpi() != (uint32_t)dpi)
                {
                    canvasSwapChain.ResizeBuffers(width, height, dpi, pixelFormat, 2);
                }
                {
                    CanvasDrawingSession outputDrawingSession = canvasSwapChain.CreateDrawingSession(winrt::Windows::UI::Colors::Transparent());

                    renderingTarget = CanvasRenderTarget(outputDrawingSession, width, height, dpi, pixelFormat, CanvasAlphaMode::Premultiplied);


                    mediaSource.RenderSubtitlesToDirectXSurface(renderingTarget, mediaSource.SubtitleStreams().GetAt(0), playbackSession.Position());

                    winrt::com_ptr<IDXGISurface> dxgiRenderTarget;
                    auto hr = GetDXGISurface(renderingTarget, dxgiRenderTarget);

                    auto bitmap = DXGISurface2CanvasBitmap(canvasDevice, dxgiRenderTarget.get());

                    outputDrawingSession.DrawImage(bitmap);
                    canvasSwapChain.Present();
                }
            }
            catch (...)
            {

            }
        }

        winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DSurface GetSurface(const winrt::com_ptr<IDXGISurface>& source)
        {
            winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DSurface result;
            winrt::com_ptr<::IInspectable> inspectableSurface;
            winrt::check_hresult(CreateDirect3D11SurfaceFromDXGISurface(source.get(), reinterpret_cast<::IInspectable**>(winrt::put_abi(inspectableSurface))));
            inspectableSurface.as(result);
            return result;
        }

        winrt::Microsoft::Graphics::Canvas::CanvasSwapChain canvasSwapChain = { nullptr };

        winrt::Windows::UI::Xaml::Controls::SwapChainPanel swapChainPanel;
        winrt::Windows::Media::Playback::MediaPlaybackSession playbackSession = { nullptr };

        winrt::Windows::UI::Xaml::DispatcherTimer timer;
        int backBufferIndex = 0;

        winrt::FFmpegInteropX::FFmpegMediaSource mediaSource = { nullptr };
        winrt::FFmpegInteropX::SubtitleStreamInfo subtitle = { nullptr };

        winrt::Microsoft::Graphics::Canvas::CanvasRenderTarget renderingTarget = { nullptr };
    };
}
namespace winrt::AssSsaRenderElement::factory_implementation
{
    struct AssSsaRenderer : AssSsaRendererT<AssSsaRenderer, implementation::AssSsaRenderer>
    {
    };
}
