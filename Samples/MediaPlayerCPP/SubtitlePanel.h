#pragma once
#include "pch.h"
#include "DirectXHelper.h"
#include "DirectXPanelBase.h"
#include "StepTimer.h"
#include <windows.graphics.directx.direct3d11.interop.h>
#include <windows.graphics.directx.direct3d11.h>
#include <ppltasks.h>

namespace DirectXPanels
{
    using namespace FFmpegInteropX;
    using namespace Windows::Media::Playback;
    using namespace Windows::System::Threading;
    using namespace Windows::Foundation;
    using namespace Microsoft::WRL;
    using namespace DX;

    // Base class for a SwapChainPanel-based DirectX rendering surface to be used in XAML apps.
    [Windows::Foundation::Metadata::WebHostHidden]
        public ref class SubtitlePanel sealed : public DirectXPanelBase
    {
    public:

        SubtitlePanel() : DirectXPanelBase()
        {
            m_alphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;

            CreateDeviceIndependentResources();
            CreateDeviceResources();
            CreateSizeDependentResources();
        }

        void SetFFmpegMediaSource(FFmpegMediaSource^ mediaSource)
        {
            source = mediaSource;
            session = mediaSource->PlaybackSession;
        }

        void SetSelectedSubtitleStream(SubtitleStreamInfo^ subtitleStream)
        {
            stream = subtitleStream;
        }

        void SetFrameRate(Windows::Foundation::TimeSpan frameRate)
        {
            this->frameRate = frameRate;
        }

        void StartRendering()
        {
            // If the animation render loop is already running then do not start another thread.
            if (m_renderLoopWorker != nullptr && m_renderLoopWorker->Status == Windows::Foundation::AsyncStatus::Started)
            {
                return;
            }

            // Create a task that will be run on a background thread.
            auto workItemHandler = ref new WorkItemHandler([this](IAsyncAction^ action)
                {
                    // Calculate the updated frame and render once per vertical blanking interval.
                    while (action->Status == Windows::Foundation::AsyncStatus::Started)
                    {
                        m_timer.Tick([&]()
                            {
                                Concurrency::critical_section::scoped_lock lock(m_criticalSection);
                                Render();
                            });

                        // Halt the thread until the next vblank is reached.
                        // This ensures the app isn't updating and rendering faster than the display can refresh,
                        // which would unnecessarily spend extra CPU and GPU resources.  This helps improve battery life.
                        m_dxgiOutput->WaitForVBlank();
                    }
                });

            // Run task on a dedicated high priority background thread.
            m_renderLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);
        }

        void StopRendering()
        {
            // Cancel the asynchronous task and let the render thread exit.

            if (m_renderLoopWorker)
                m_renderLoopWorker->Cancel();
        }

        long long GetFrameCount()
        {
            return m_timer.GetFrameCount();
        }

    protected private:
        virtual void Render() override
        {
            if (!m_loadingComplete || !source || !stream || !session) {
                return;
            }

            /*const FLOAT transparent[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
            const FLOAT color[4] = { 0.5f, 0.5f, 0.5f, 0.5f };
            m_d3dContext->ClearRenderTargetView(renderTargetView.Get(), color);*/

            auto dxSurface = GetSurface(m_surface);
            source->RenderSubtitlesToDirectXSurface(dxSurface, stream, session->Position);

            Present();
        }

        Windows::Graphics::DirectX::Direct3D11::IDirect3DSurface^ GetSurface(const ComPtr<IDXGISurface> dxgiSurface)
        {
            ComPtr<ABI::Windows::Graphics::DirectX::Direct3D11::IDirect3DSurface> surface;
            ComPtr<IInspectable> inspectableSurface;
            ThrowIfFailed(CreateDirect3D11SurfaceFromDXGISurface(dxgiSurface.Get(), &inspectableSurface));
            inspectableSurface.As(&surface);
            return reinterpret_cast<Windows::Graphics::DirectX::Direct3D11::IDirect3DSurface^>(surface.Get());
        }

        virtual void CreateSizeDependentResources() override
        {
            renderTargetView = nullptr;

            DirectXPanelBase::CreateSizeDependentResources();

            // Create a render target view of the swap chain back buffer.
            ComPtr<ID3D11Texture2D> backBuffer;
            ThrowIfFailed(
                m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer))
            );

            // Create render target view.
            ThrowIfFailed(
                m_d3dDevice->CreateRenderTargetView(
                    backBuffer.Get(),
                    nullptr,
                    &renderTargetView)
            );
        }

        virtual void CreateDeviceResources() override
        {
            DirectXPanelBase::CreateDeviceResources();

            // Retrieve DXGIOutput representing the main adapter output.
            ComPtr<IDXGIFactory1> dxgiFactory;
            ThrowIfFailed(
                CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory))
            );

            ComPtr<IDXGIAdapter> dxgiAdapter;
            ThrowIfFailed(
                dxgiFactory->EnumAdapters(0, &dxgiAdapter)
            );

            ThrowIfFailed(
                dxgiAdapter->EnumOutputs(0, &m_dxgiOutput)
            );

            m_loadingComplete = true;
        }

    private:

        FFmpegMediaSource^ source;
        MediaPlaybackSession^ session;
        SubtitleStreamInfo^ stream;

        Windows::Foundation::TimeSpan frameRate;
        Windows::Foundation::TimeSpan lastPosition;

        ComPtr<IDXGIOutput> m_dxgiOutput;
        ComPtr<ID3D11RenderTargetView> renderTargetView;

        Windows::Foundation::IAsyncAction^ m_renderLoopWorker;
        // Rendering loop timer.
        DX::StepTimer                                       m_timer;
    };
}
