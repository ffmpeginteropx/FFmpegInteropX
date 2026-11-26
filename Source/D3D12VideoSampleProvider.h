#include "pch.h"
#include "UncompressedVideoSampleProvider.h"
#include "TexturePool.h"
#include "DirectXInteropHelper.h"
#include "FFmpegReader.h"
#include "FFmpegUtils.h"

extern "C"
{
#include <libavutil/hwcontext_d3d12va.h>
}

using namespace winrt::Windows::Media::Core;

class D3D12VideoSampleProvider : public UncompressedVideoSampleProvider, public std::enable_shared_from_this<D3D12VideoSampleProvider>
{
public:
    D3D12VideoSampleProvider(
        std::shared_ptr<FFmpegReader> reader,
        AVFormatContext* avFormatCtx,
        AVCodecContext* avCodecCtx,
        winrt::FFmpegInteropX::MediaSourceConfig const& config,
        int streamIndex,
        HardwareDecoderStatus hardwareDecoderStatus,
        bool applyHdrColorInfo)
        : UncompressedVideoSampleProvider(reader, avFormatCtx, avCodecCtx, config, streamIndex, hardwareDecoderStatus, applyHdrColorInfo)
    {
        decoder = DecoderEngine::FFmpegD3D12HardwareDecoder;
        hwCodec = avCodecCtx->codec;
    }

    virtual ~D3D12VideoSampleProvider()
    {
        ReleaseTrackedSamples();
    }

    virtual void Flush(bool flushBuffers) override
    {
        UncompressedVideoSampleProvider::Flush(flushBuffers);
        ReturnTrackedSamples();
    }

    virtual HRESULT CreateBufferFromFrame(IBuffer* pBuffer, IDirect3DSurface* surface, AVFrame* avFrame, int64_t& framePts, int64_t& frameDuration) override
    {
        HRESULT hr = S_OK;

        if (avFrame->format == AV_PIX_FMT_D3D12)
        {
            if (!m_pAvCodecCtx->hw_device_ctx)
            {
                return E_FAIL;
            }

            auto decodedFrame = reinterpret_cast<AVD3D12VAFrame*>(avFrame->data[0]);
            auto d3d12Context = (AVD3D12VADeviceContext*)((AVHWDeviceContext*)m_pAvCodecCtx->hw_device_ctx->data)->hwctx;
            if (!decodedFrame || !decodedFrame->texture || !decodedFrame->sync_ctx.fence || !decodedFrame->sync_ctx.event)
            {
                return E_FAIL;
            }

            if (decodedFrame->sync_ctx.fence->GetCompletedValue() < decodedFrame->sync_ctx.fence_value)
            {
                hr = decodedFrame->sync_ctx.fence->SetEventOnCompletion(decodedFrame->sync_ctx.fence_value, decodedFrame->sync_ctx.event);
                if (SUCCEEDED(hr) && WaitForSingleObject(decodedFrame->sync_ctx.event, INFINITE) != WAIT_OBJECT_0)
                {
                    hr = E_FAIL;
                }
            }

            winrt::com_ptr<ID3D11Resource> wrappedResource;
            if (SUCCEEDED(hr))
            {
                D3D11_RESOURCE_FLAGS resourceFlags{};
                hr = d3d11On12Device->CreateWrappedResource(
                    decodedFrame->texture,
                    &resourceFlags,
                    D3D12_RESOURCE_STATE_COMMON,
                    D3D12_RESOURCE_STATE_COMMON,
                    IID_PPV_ARGS(wrappedResource.put()));
            }

            if (SUCCEEDED(hr) && !texturePool)
            {
                std::lock_guard lock(samplesMutex);
                texturePool = std::unique_ptr<TexturePool>(new TexturePool(device, 5));
            }

            if (SUCCEEDED(hr))
            {
                ID3D11Resource* wrappedResources[] = { wrappedResource.get() };
                d3d11On12Device->AcquireWrappedResources(wrappedResources, 1);

                auto decodedTexture = wrappedResource.as<ID3D11Texture2D>();
                winrt::com_ptr<ID3D11Texture2D> renderTexture;
                {
                    std::lock_guard lock(samplesMutex);
                    hr = texturePool->GetCopyTexture(decodedTexture.get(), renderTexture);
                }
                if (SUCCEEDED(hr))
                {
                    deviceContext->CopySubresourceRegion(renderTexture.get(), 0, 0, 0, 0, decodedTexture.get(), decodedFrame->subresource_index, nullptr);
                }
               
                d3d11On12Device->ReleaseWrappedResources(wrappedResources, 1);
                deviceContext->Flush();

                if (SUCCEEDED(hr))
                {
                    auto finalSurface = renderTexture.as<IDXGISurface>();
                    *surface = DirectXInteropHelper::GetSurface(finalSurface);
                    CheckFrameSize(avFrame);
                    ReadFrameProperties(avFrame, framePts);
                }
            }
        }
        else
        {
            hr = UncompressedVideoSampleProvider::CreateBufferFromFrame(pBuffer, surface, avFrame, framePts, frameDuration);
            if (decoder != DecoderEngine::FFmpegSoftwareDecoder)
            {
                decoder = DecoderEngine::FFmpegSoftwareDecoder;
                VideoInfo().as<implementation::VideoStreamInfo>()->decoderEngine = decoder;
            }
        }

        return hr;
    }

    virtual HRESULT SetSampleProperties(MediaStreamSample const& sample) override
    {
        if (sample.Direct3D11Surface())
        {
            std::lock_guard lock(samplesMutex);
            auto sampleNative = sample.as<winrt::Windows::Foundation::IUnknown>();
            auto token = sample.Processed(weak_handler(this, &D3D12VideoSampleProvider::OnProcessed));
            trackedSamples.insert_or_assign(sampleNative, token);
        }

        return UncompressedVideoSampleProvider::SetSampleProperties(sample);
    }

    void OnProcessed(MediaStreamSample const& sender, winrt::Windows::Foundation::IInspectable const&)
    {
        std::lock_guard lock(samplesMutex);
        auto sampleNative = sender.as<winrt::Windows::Foundation::IUnknown>();
        auto mapEntry = trackedSamples.find(sampleNative);
        if (mapEntry != trackedSamples.end())
        {
            sender.Processed(mapEntry->second);
            trackedSamples.erase(mapEntry);
            ReturnTextureToPool(sender);
        }
    }

    virtual HRESULT SetHardwareDevice(AVBufferRef* avHardwareContextD3D11, AVBufferRef* avHardwareContextD3D12) override
    {
        UNREFERENCED_PARAMETER(avHardwareContextD3D11);
        if (!avHardwareContextD3D12)
        {
            return E_INVALIDARG;
        }

        auto d3d12DataBuffer = (AVHWDeviceContext*)avHardwareContextD3D12->data;
        auto d3d12Context = (AVD3D12VADeviceContext*)d3d12DataBuffer->hwctx;
        if (d3d12DataBuffer->type != AV_HWDEVICE_TYPE_D3D12VA || !d3d12Context->device)
        {
            return E_FAIL;
        }

        D3D12_COMMAND_QUEUE_DESC queueDesc{};
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        winrt::com_ptr<ID3D12CommandQueue> newQueue;
        HRESULT hr = d3d12Context->device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(newQueue.put()));

        winrt::com_ptr<ID3D11Device> newDevice;
        winrt::com_ptr<ID3D11DeviceContext> newDeviceContext;
        if (SUCCEEDED(hr))
        {
            ::IUnknown* queues[] = { newQueue.get() };
            D3D_FEATURE_LEVEL featureLevel{};
            hr = D3D11On12CreateDevice(
                d3d12Context->device,
                D3D11_CREATE_DEVICE_VIDEO_SUPPORT,
                nullptr,
                0,
                queues,
                ARRAYSIZE(queues),
                0,
                newDevice.put(),
                newDeviceContext.put(),
                &featureLevel);
        }

        winrt::com_ptr<ID3D11On12Device> newD3D11On12Device;
        if (SUCCEEDED(hr))
        {
            newD3D11On12Device = newDevice.try_as<ID3D11On12Device>();
            if (!newD3D11On12Device)
            {
                hr = E_NOINTERFACE;
            }
        }

        if (FAILED(hr))
        {
            return hr;
        }

        bool needsReinit = device.get() != nullptr;
        ReleaseTrackedSamples();
        {
            std::lock_guard lock(samplesMutex);
            texturePool.reset();
        }

        device = newDevice;
        deviceContext = newDeviceContext;
        d3d11On12Device = newD3D11On12Device;
        d3d12Queue = newQueue;

        if (needsReinit)
        {
            hr = UpdateCodecContext(hwCodec, avHardwareContextD3D12);
        }
        else if (!m_pAvCodecCtx->hw_device_ctx || m_pAvCodecCtx->hw_device_ctx->data != avHardwareContextD3D12->data)
        {
            av_buffer_unref(&m_pAvCodecCtx->hw_device_ctx);
            m_pAvCodecCtx->hw_device_ctx = av_buffer_ref(avHardwareContextD3D12);
        }

        return hr;
    }

    HRESULT UpdateCodecContext(const AVCodec* codec, AVBufferRef* avHardwareContext)
    {
        HRESULT hr = S_OK;
        avcodec_free_context(&m_pAvCodecCtx);
        m_pAvCodecCtx = avcodec_alloc_context3(codec);
        if (!m_pAvCodecCtx)
        {
            hr = E_OUTOFMEMORY;
        }

        if (SUCCEEDED(hr))
        {
            if (!avHardwareContext)
            {
                unsigned threads = std::thread::hardware_concurrency();
                m_pAvCodecCtx->thread_count = m_config.Video().MaxDecoderThreads() == 0 ? threads : min(threads, m_config.Video().MaxDecoderThreads());
                m_pAvCodecCtx->thread_type = m_config.as<implementation::MediaSourceConfig>()->IsFrameGrabber ? FF_THREAD_SLICE : FF_THREAD_FRAME | FF_THREAD_SLICE;
            }
            hr = avcodec_parameters_to_context(m_pAvCodecCtx, m_pAvStream->codecpar);
        }

        if (SUCCEEDED(hr))
        {
            m_pAvCodecCtx->hw_device_ctx = avHardwareContext ? av_buffer_ref(avHardwareContext) : nullptr;
            m_pAvCodecCtx->get_format = &FFmpegUtils::get_format;
            hr = avcodec_open2(m_pAvCodecCtx, codec, nullptr);
        }

        if (SUCCEEDED(hr))
        {
            frameProvider->UpdateCodecContext(m_pAvCodecCtx);
        }
        return hr;
    }

    static HRESULT InitializeHardwareDeviceContext(
        AVBufferRef* avHardwareContext,
        const winrt::com_ptr<IMFDXGIDeviceManager>& deviceManager)
    {
        winrt::com_ptr<ID3D12Device> d3d12Device;
        HRESULT hr = DirectXInteropHelper::GetD3D12DeviceFromStreamSource(deviceManager, d3d12Device);

        if (SUCCEEDED(hr))
        {
            auto internalContext = (AVD3D12VADeviceContext*)((AVHWDeviceContext*)avHardwareContext->data)->hwctx;
            d3d12Device.copy_to(&internalContext->device);
            hr = av_hwdevice_ctx_init(avHardwareContext);
        }
        return hr;
    }

private:
    void ReturnTextureToPool(MediaStreamSample const& sample)
    {
        winrt::com_ptr<IDXGISurface> surface;
        HRESULT hr = DirectXInteropHelper::GetDXGISurface(sample.Direct3D11Surface(), surface);
        if (SUCCEEDED(hr))
        {
            texturePool->ReturnTexture(surface.as<ID3D11Texture2D>());
        }
    }

    void ReleaseTrackedSamples()
    {
        std::lock_guard lock(samplesMutex);
        for (auto& entry : trackedSamples)
        {
            auto sampleNative = entry.first;
            auto sample = sampleNative.as<MediaStreamSample>();
            sample.Processed(entry.second);
        }
        trackedSamples.clear();
    }

    void ReturnTrackedSamples()
    {
        std::lock_guard lock(samplesMutex);
        for (auto& entry : trackedSamples)
        {
            auto sampleNative = entry.first;
            auto sample = sampleNative.as<MediaStreamSample>();
            sample.Processed(entry.second);
            ReturnTextureToPool(sample);
        }
        trackedSamples.clear();
    }

    const AVCodec* hwCodec = nullptr;
    winrt::com_ptr<ID3D11On12Device> d3d11On12Device;
    winrt::com_ptr<ID3D12CommandQueue> d3d12Queue;
    std::unique_ptr<TexturePool> texturePool;
    std::map<winrt::Windows::Foundation::IUnknown, winrt::event_token> trackedSamples;
    std::mutex samplesMutex;
};
