//*****************************************************************************
//
//	Copyright 2015 Microsoft Corporation
//
//	Licensed under the Apache License, Version 2.0 (the "License");
//	you may not use this file except in compliance with the License.
//	You may obtain a copy of the License at
//
//	http ://www.apache.org/licenses/LICENSE-2.0
//
//	Unless required by applicable law or agreed to in writing, software
//	distributed under the License is distributed on an "AS IS" BASIS,
//	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//	See the License for the specific language governing permissions and
//	limitations under the License.
//
//*****************************************************************************

#pragma once
#include "pch.h"


using namespace winrt::Windows::Storage::Streams;
using namespace winrt::Windows::Media::MediaProperties;
using namespace winrt::Windows::Graphics::DirectX::Direct3D11;
using namespace std;

using namespace winrt::FFmpegInteropX;

class FFmpegReader;
class StreamBuffer;

class MediaSampleProvider
{
public:
    //----------------------------------------------------------------------
    AVStream* m_pAvStream = NULL;
    INT64 m_startOffset = 0;
    std::vector<winrt::hstring> _properties = std::vector<winrt::hstring>();
    //----------------------------------------------------------------------

    virtual ~MediaSampleProvider();
    virtual winrt::Windows::Media::Core::MediaStreamSample GetNextSample();
    virtual void Flush(bool flushBuffers);

    winrt::Windows::Media::Core::IMediaStreamDescriptor StreamDescriptor()
    {
        return m_streamDescriptor;
    }

    winrt::Windows::Media::Core::VideoStreamDescriptor VideoDescriptor()
    {
        return m_streamDescriptor.as<winrt::Windows::Media::Core::VideoStreamDescriptor>();
    }

    winrt::Windows::Media::Core::AudioStreamDescriptor AudioDescriptor()
    {
        return m_streamDescriptor.as<winrt::Windows::Media::Core::AudioStreamDescriptor>();
    }

    IStreamInfo StreamInfo()
    {
        return streamInfo;
    }

    AudioStreamInfo AudioInfo()
    {
        return streamInfo.as<AudioStreamInfo>();
    }

    VideoStreamInfo VideoInfo()
    {
        return streamInfo.as<VideoStreamInfo>();
    }

    SubtitleStreamInfo SubtitleInfo()
    {
        return streamInfo.as<SubtitleStreamInfo>();
    }

    int StreamIndex()
    {
        return m_streamIndex;
    }

    bool IsEnabled()
    {
        return m_isEnabled;
    }

    HardwareDecoderStatus HardwareAccelerationStatus()
    {
        return hardwareDecoderStatus;
    }

    DecoderEngine Decoder()
    {
        return decoder;
    }

    bool IsCleanSample = false;

    winrt::hstring Name{};
    winrt::hstring Language{};
    winrt::hstring CodecName{};
    TimeSpan LastSampleTimestamp{};

    virtual HRESULT Initialize();
    void InitializeNameLanguageCodec();
    virtual void InitializeStreamInfo();
    virtual void QueuePacket(AVPacket* packet);
    HRESULT GetNextPacket(AVPacket** avPacket, LONGLONG& packetPts, LONGLONG& packetDuration);
    bool IsBufferFull();
    virtual HRESULT CreateNextSampleBuffer(winrt::Windows::Storage::Streams::IBuffer* pBuffer, int64_t& samplePts, int64_t& sampleDuration, winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DSurface* surface) = 0;
    HRESULT GetNextPacketTimestamp(TimeSpan& timestamp, TimeSpan& packetDuration);
    HRESULT SkipPacketsUntilTimestamp(TimeSpan const& timestamp);
    virtual winrt::Windows::Media::Core::IMediaStreamDescriptor CreateStreamDescriptor() = 0;
    virtual HRESULT SetSampleProperties(winrt::Windows::Media::Core::MediaStreamSample const& sample) { UNREFERENCED_PARAMETER(sample); return S_OK; }; // can be overridded for setting extended properties
    virtual void EnableStream();
    virtual void DisableStream();
    virtual void SetFilters(winrt::hstring filterDefinition) { };// override for setting effects in sample providers
    virtual void DisableFilters() {};//override for disabling filters in sample providers;
    virtual void SetCommonVideoEncodingProperties(winrt::Windows::Media::MediaProperties::VideoEncodingProperties const& videoEncodingProperties, bool isCompressedFormat);
    virtual void Detach();

    virtual HRESULT SetHardwareDevice(winrt::com_ptr<ID3D11Device> newDevice,
        winrt::com_ptr<ID3D11DeviceContext> newDeviceContext,
        AVBufferRef* avHardwareContext)
    {
        UNREFERENCED_PARAMETER(newDevice);
        UNREFERENCED_PARAMETER(newDeviceContext);
        UNREFERENCED_PARAMETER(avHardwareContext);
        return S_OK;
    };

    void FreeHardwareDevice()
    {
        if (m_pAvCodecCtx->hw_device_ctx)
        {
            av_buffer_unref(&m_pAvCodecCtx->hw_device_ctx);
        }
        device = nullptr;
        deviceContext = nullptr;
    }

    virtual void NotifyCreateSource()
    {
        if (m_pAvFormatCtx->start_time != AV_NOPTS_VALUE)
        {
            auto streamStartTime = ConvertDuration(m_pAvStream->start_time);

            if (m_pAvFormatCtx->start_time == streamStartTime.count() / 10)
            {
                // use more precise start time
                m_startOffset = streamStartTime.count();
            }
            else
            {
                m_startOffset = m_pAvFormatCtx->start_time * 10;
            }
        }
    }

    TimeSpan ConvertPosition(LONGLONG pts)
    {
        return TimeSpan{ LONGLONG(timeBaseFactor * pts) - m_startOffset };
    }

    LONGLONG ConvertPosition(TimeSpan position)
    {
        return LONGLONG((position.count() + m_startOffset) / timeBaseFactor);
    }

    TimeSpan ConvertDuration(LONGLONG duration)
    {
        return TimeSpan{ LONGLONG(timeBaseFactor * duration) };
    }

    LONGLONG ConvertDuration(TimeSpan duration)
    {
        return LONGLONG(duration.count() / timeBaseFactor);
    }

protected:
    MediaSampleProvider(
        std::shared_ptr<FFmpegReader> reader,
        AVFormatContext* avFormatCtx,
        AVCodecContext* avCodecCtx,
        MediaSourceConfig const& config,
        int streamIndex,
        HardwareDecoderStatus hardwareDecoderStatus);

private:
    INT64 m_nextPacketPts = 0;
    winrt::Windows::Media::Core::IMediaStreamDescriptor m_streamDescriptor = nullptr;
    HardwareDecoderStatus hardwareDecoderStatus;

public:
    std::shared_ptr<StreamBuffer> packetBuffer;


protected:
    // The FFmpeg context. Because they are complex types
    // we declare them as internal so they don't get exposed
    // externally
    MediaSourceConfig m_config = nullptr;
    std::shared_ptr<FFmpegReader> m_pReader;
    AVFormatContext* m_pAvFormatCtx = NULL;
    AVCodecContext* m_pAvCodecCtx = NULL;
    IStreamInfo streamInfo = nullptr;
    bool m_isEnabled = false;
    bool m_isDiscontinuous = false;
    int m_streamIndex = 0;
    double timeBaseFactor = 0;
    DecoderEngine decoder = DecoderEngine::FFmpegSoftwareDecoder;
    winrt::com_ptr<ID3D11Device> device = NULL;
    winrt::com_ptr<ID3D11DeviceContext> deviceContext = NULL;

};

// free AVBufferRef*
void free_buffer(void* lpVoid);
