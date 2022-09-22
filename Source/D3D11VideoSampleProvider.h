#pragma once
#include "pch.h"
#include "UncompressedVideoSampleProvider.h"
#include "TexturePool.h"
#include "DirectXInteropHelper.h"
#include "FFmpegReader.h"
#include "FFmpegUtils.h"

extern "C"
{
#include <libavutil/hwcontext_d3d11va.h>
}

using namespace winrt::Windows::Media::Core;

class D3D11VideoSampleProvider : public UncompressedVideoSampleProvider, public std::enable_shared_from_this<D3D11VideoSampleProvider>
{
private:
    const AVCodec* hwCodec;
    const AVCodec* swCodec;
public:

    D3D11VideoSampleProvider(
        std::shared_ptr<FFmpegReader> reader,
        AVFormatContext* avFormatCtx,
        AVCodecContext* avCodecCtx,
        winrt::FFmpegInteropX::MediaSourceConfig const& config,
        int streamIndex,
        HardwareDecoderStatus hardwareDecoderStatus)
        : UncompressedVideoSampleProvider(reader, avFormatCtx, avCodecCtx, config, streamIndex, hardwareDecoderStatus)
    {
        decoder = DecoderEngine::FFmpegD3D11HardwareDecoder;
        hwCodec = avCodecCtx->codec;

        if (avCodecCtx->codec_id == AVCodecID::AV_CODEC_ID_AV1)
        {
            swCodec = avcodec_find_decoder_by_name("libdav1d");
        }
        else
        {
            swCodec = NULL;
        }
    }

public:

    virtual ~D3D11VideoSampleProvider()
    {
        ReleaseTrackedSamples();
    }

    virtual void Flush() override
    {
        UncompressedVideoSampleProvider::Flush();
        ReturnTrackedSamples();
    }


    virtual HRESULT CreateBufferFromFrame(IBuffer* pBuffer, IDirect3DSurface* surface, AVFrame* avFrame, int64_t& framePts, int64_t& frameDuration) override
    {
        HRESULT hr = S_OK;

        if (avFrame->format == AV_PIX_FMT_D3D11)
        {
            if (!texturePool)
            {
                // init texture pool, fail if we did not get a device ptr
                if (device && deviceContext)
                {
                    texturePool = std::unique_ptr<TexturePool>(new TexturePool(device, 5));
                }
                else
                {
                    hr = E_FAIL;
                }
            }
            //copy texture data
            if (SUCCEEDED(hr))
            {
                //cast the AVframe to texture 2D
                auto decodedTexture = reinterpret_cast<ID3D11Texture2D*>(avFrame->data[0]);
                winrt::com_ptr<ID3D11Texture2D> renderTexture;
                //happy path:decoding and rendering on same GPU
                hr = texturePool->GetCopyTexture(decodedTexture, renderTexture);
                deviceContext->CopySubresourceRegion(renderTexture.get(), 0, 0, 0, 0, decodedTexture, (UINT)(unsigned long long)avFrame->data[1], NULL);
                deviceContext->Flush();

                //create a IDXGISurface from the shared texture
                auto finalSurface = renderTexture.as<IDXGISurface>();

                //get the IDirect3DSurface pointer
                if (SUCCEEDED(hr))
                {
                    *surface = DirectXInteropHelper::GetSurface(finalSurface);
                }

                if (SUCCEEDED(hr))
                {
                    CheckFrameSize(avFrame);
                    ReadFrameProperties(avFrame, framePts);
                }

                if (decoder != DecoderEngine::FFmpegD3D11HardwareDecoder)
                {
                    decoder = DecoderEngine::FFmpegD3D11HardwareDecoder;
                    VideoInfo().as<implementation::VideoStreamInfo>()->decoderEngine = decoder;
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

            // AddRef the sample on native interface to prevent it from being collected before Processed is called
            auto sampleNative = sample.as<winrt::Windows::Foundation::IUnknown>();

            // Attach Processed event and store in samples list
            auto token = sample.Processed(weak_handler(this, &D3D11VideoSampleProvider::OnProcessed));
            trackedSamples.insert_or_assign(sampleNative, token);
        }

        return UncompressedVideoSampleProvider::SetSampleProperties(sample);
    };

    void OnProcessed(MediaStreamSample const& sender, winrt::Windows::Foundation::IInspectable const&)
    {
        std::lock_guard lock(samplesMutex);

        auto sampleNative = sender.as<winrt::Windows::Foundation::IUnknown>();
        auto mapEntry = trackedSamples.find(sampleNative);
        if (mapEntry == trackedSamples.end())
        {
            // sample was already released during Flush() or destructor
        }
        else
        {
            // Release the sample's native interface and return texture to pool
            sender.Processed(mapEntry->second);
            trackedSamples.erase(mapEntry);

            ReturnTextureToPool(sender);
        }
    }

    void ReturnTextureToPool(MediaStreamSample const& sample)
    {
        winrt::com_ptr<IDXGISurface> surface;
        HRESULT hr = DirectXInteropHelper::GetDXGISurface(sample.Direct3D11Surface(), surface);

        if (SUCCEEDED(hr))
        {
            auto texture = surface.as<ID3D11Texture2D>();
            texturePool->ReturnTexture(texture);
        }
    }

    void ReleaseTrackedSamples()
    {
        std::lock_guard lock(samplesMutex);
        for (auto& entry : trackedSamples)
        {
            // detach Processed event and release native interface
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
            // detach Processed event and release native interface
            auto sampleNative = entry.first;
            auto sample = sampleNative.as<MediaStreamSample>();

            sample.Processed(entry.second);

            // return texture to pool
            ReturnTextureToPool(sample);
        }

        trackedSamples.clear();
    }

    virtual HRESULT SetHardwareDevice(winrt::com_ptr<ID3D11Device> newDevice,
        winrt::com_ptr<ID3D11DeviceContext> newDeviceContext,
        AVBufferRef* avHardwareContext) override
    {
        HRESULT hr = S_OK;

        bool isCompatible = false;
        hr = CheckHWAccelerationCompatible(newDevice, isCompatible);

        if (SUCCEEDED(hr))
        {
            if (isCompatible)
            {
                bool needsReinit = device.get();

                // must explicitly reset com_ptr before assigning new value
                device = nullptr;
                deviceContext = nullptr;

                device = newDevice;
                deviceContext = newDeviceContext;

                if (needsReinit)
                {
                    hr = UpdateCodecContext(hwCodec, avHardwareContext);
                }
                else if (!m_pAvCodecCtx->hw_device_ctx || m_pAvCodecCtx->hw_device_ctx->data != avHardwareContext->data)
                {
                    av_buffer_unref(&m_pAvCodecCtx->hw_device_ctx);
                    m_pAvCodecCtx->hw_device_ctx = av_buffer_ref(avHardwareContext);
                }
            }
            else
            {
                device = nullptr;
                deviceContext = nullptr;
                av_buffer_unref(&m_pAvCodecCtx->hw_device_ctx);

                if (swCodec)
                {
                    hr = UpdateCodecContext(swCodec, NULL);
                }
                else
                {
                    hr = E_FAIL;
                }
            }
        }

        texturePool.reset();
        texturePool = nullptr;

        ReleaseTrackedSamples();

        return hr;
    }

    HRESULT UpdateCodecContext(const AVCodec* codec, AVBufferRef* avHardwareContext)
    {
        auto hr = S_OK;

        avcodec_free_context(&m_pAvCodecCtx);

        m_pAvCodecCtx = avcodec_alloc_context3(codec);
        if (!m_pAvCodecCtx)
        {
            hr = E_OUTOFMEMORY;
        }

        if (SUCCEEDED(hr))
        {
            // enable multi threading only for SW decoders
            if (!avHardwareContext)
            {
                unsigned threads = std::thread::hardware_concurrency();
                m_pAvCodecCtx->thread_count = m_config.MaxVideoThreads() == 0 ? threads : min(threads, m_config.MaxVideoThreads());
                m_pAvCodecCtx->thread_type = m_config.as<implementation::MediaSourceConfig>()->IsFrameGrabber ? FF_THREAD_SLICE : FF_THREAD_FRAME | FF_THREAD_SLICE;
            }

            // initialize the stream parameters with demuxer information
            hr = avcodec_parameters_to_context(m_pAvCodecCtx, m_pAvStream->codecpar);
        }

        if (SUCCEEDED(hr))
        {
            m_pAvCodecCtx->hw_device_ctx = avHardwareContext ? av_buffer_ref(avHardwareContext) : NULL;
            m_pAvCodecCtx->get_format = &FFmpegUtils::get_format;
            hr = avcodec_open2(m_pAvCodecCtx, codec, NULL);
        }

        if (SUCCEEDED(hr))
        {
            frameProvider->UpdateCodecContext(m_pAvCodecCtx);
        }

        return hr;
    }

    HRESULT CheckHWAccelerationCompatible(winrt::com_ptr<ID3D11Device> newDevice, bool& isCompatible)
    {
        HRESULT hr = S_OK;
        isCompatible = true;

        if (m_pAvCodecCtx->codec->id == AVCodecID::AV_CODEC_ID_AV1)
        {
            isCompatible = m_pAvCodecCtx->profile >= 0 && m_pAvCodecCtx->profile <= 2;
            if (isCompatible)
            {
                const GUID av1Profiles[3]{
                    D3D11_DECODER_PROFILE_AV1_VLD_PROFILE0,
                    D3D11_DECODER_PROFILE_AV1_VLD_PROFILE1,
                    D3D11_DECODER_PROFILE_AV1_VLD_PROFILE2
                };
                auto requiredProfile = av1Profiles[m_pAvCodecCtx->profile];
                auto videoDevice = newDevice.try_as<ID3D11VideoDevice>();

                // check profile exists
                if (videoDevice)
                {
                    isCompatible = false;
                    GUID profile;
                    int count = videoDevice->GetVideoDecoderProfileCount();
                    for (int i = 0; i < count; i++)
                    {
                        hr = videoDevice->GetVideoDecoderProfile(i, &profile);
                        if (FAILED(hr))
                        {
                            break;
                        }
                        else if (profile == requiredProfile)
                        {
                            isCompatible = true;
                            break;
                        }
                    }
                }

                // check resolution
                if (SUCCEEDED(hr) && isCompatible)
                {
                    D3D11_VIDEO_DECODER_DESC desc;
                    desc.Guid = requiredProfile;
                    desc.OutputFormat = DXGI_FORMAT_NV12;

                    desc.SampleWidth = m_pAvCodecCtx->coded_width;
                    desc.SampleHeight = m_pAvCodecCtx->coded_height;

                    UINT count;
                    hr = videoDevice->GetVideoDecoderConfigCount(&desc, &count);
                    if (SUCCEEDED(hr))
                    {
                        isCompatible = count > 0;
                    }
                }

            }

            if (FAILED(hr))
            {
                isCompatible = false;
            }
        }

        return hr;
    }

    static HRESULT InitializeHardwareDeviceContext(MediaStreamSource sender,
        AVBufferRef* avHardwareContext,
        winrt::com_ptr<ID3D11Device>& outDevice,
        winrt::com_ptr<ID3D11DeviceContext>& outDeviceContext,
        const winrt::com_ptr<IMFDXGIDeviceManager>& deviceManager,
        HANDLE* outDeviceHandle)
    {
        winrt::com_ptr<ID3D11Device> device;
        winrt::com_ptr<ID3D11DeviceContext> deviceContext;
        winrt::com_ptr<ID3D11VideoDevice> videoDevice;
        winrt::com_ptr<ID3D11VideoContext> videoContext;

        HRESULT hr = DirectXInteropHelper::GetDeviceFromStreamSource(deviceManager, device, deviceContext, videoDevice, outDeviceHandle);

        if (SUCCEEDED(hr)) hr = deviceContext->QueryInterface(videoContext.put());

        auto dataBuffer = (AVHWDeviceContext*)avHardwareContext->data;
        auto internalDirectXHwContext = (AVD3D11VADeviceContext*)dataBuffer->hwctx;

        if (SUCCEEDED(hr))
        {
            // give ownership to FFmpeg

            device.copy_to(&internalDirectXHwContext->device);
            deviceContext.copy_to(&internalDirectXHwContext->device_context);
            videoDevice.copy_to(&internalDirectXHwContext->video_device);
            videoContext.copy_to(&internalDirectXHwContext->video_context);
        }
        else
        {
            // release
            device = nullptr;
            deviceContext = nullptr;
            videoDevice = nullptr;
            videoContext = nullptr;
        }

        if (SUCCEEDED(hr))
        {
            // multithread interface seems to be optional
            auto multithread = device.try_as<ID3D10Multithread>();
            if (multithread)
                multithread->SetMultithreadProtected(TRUE);
        }

        if (SUCCEEDED(hr))
        {
            hr = av_hwdevice_ctx_init(avHardwareContext);
        }

        if (SUCCEEDED(hr))
        {
            // addref and hand out pointers
            outDevice = device;
            outDeviceContext = deviceContext;
        }

        return hr;
    }

    std::unique_ptr<TexturePool> texturePool;
    std::map<winrt::Windows::Foundation::IUnknown, winrt::event_token> trackedSamples;
    std::mutex samplesMutex;
};

