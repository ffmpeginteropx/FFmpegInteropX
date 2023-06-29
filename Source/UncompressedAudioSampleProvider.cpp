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

#include "pch.h"

#include <mfapi.h>

#include "UncompressedAudioSampleProvider.h"
#include "NativeBufferFactory.h"
#include "AudioEffectFactory.h"
#include "AvCodecContextHelpers.h"

extern "C"
{
#include <libswresample/swresample.h>
}

UncompressedAudioSampleProvider::UncompressedAudioSampleProvider(
    std::shared_ptr<FFmpegReader> reader,
    AVFormatContext* avFormatCtx,
    AVCodecContext* avCodecCtx,
    winrt::FFmpegInteropX::MediaSourceConfig const& config,
    int streamIndex)
    : UncompressedSampleProvider(reader, avFormatCtx, avCodecCtx, config, streamIndex, HardwareDecoderStatus::Unknown)
    , m_pSwrCtx(nullptr)
{
}

winrt::Windows::Media::Core::IMediaStreamDescriptor UncompressedAudioSampleProvider::CreateStreamDescriptor()
{
    frameProvider = std::shared_ptr<UncompressedFrameProvider>(new UncompressedFrameProvider(m_pAvFormatCtx, m_pAvCodecCtx, std::shared_ptr<AudioEffectFactory>(new AudioEffectFactory(m_pAvCodecCtx))));

    auto format = m_pAvCodecCtx->sample_fmt != AV_SAMPLE_FMT_NONE ? m_pAvCodecCtx->sample_fmt : AV_SAMPLE_FMT_S16;
    auto sampleRate = m_pAvCodecCtx->sample_rate;

    inSampleFormat = format;
    inSampleRate = outSampleRate = sampleRate;

    outSampleFormat = av_get_packed_sample_fmt(inSampleFormat);

    // replace unsupported formats with closest format
    if (outSampleFormat == AV_SAMPLE_FMT_DBL)
        outSampleFormat = AV_SAMPLE_FMT_FLT;
    if (outSampleFormat == AV_SAMPLE_FMT_S64)
        outSampleFormat = AV_SAMPLE_FMT_S32;

    inChannelLayout = m_pAvCodecCtx->ch_layout;
    outChannelLayout = m_pAvCodecCtx->ch_layout;

    if (m_pAvCodecCtx->profile == FF_PROFILE_AAC_HE_V2 && m_pAvCodecCtx->ch_layout.nb_channels == 1)
    {
        inChannelLayout.nb_channels = 2;
        inChannelLayout.order = AV_CHANNEL_ORDER_NATIVE;
        inChannelLayout.u.mask = AV_CH_LAYOUT_STEREO;

        outChannelLayout = inChannelLayout;
    }

    auto nativeLayout = AvCodecContextHelpers::GetChannelLayout(m_pAvCodecCtx);
    if (m_config.DownmixAudioStreamsToStereo() && nativeLayout > AV_CH_LAYOUT_STEREO)
    {
        // use existing downmix channels, if available, otherwise perform manual downmix using resampler
        outChannelLayout.order = AV_CHANNEL_ORDER_NATIVE;
        if (nativeLayout & AV_CH_LAYOUT_STEREO_DOWNMIX)
        {
            outChannelLayout.u.mask = AV_CH_LAYOUT_STEREO_DOWNMIX;
            outChannelLayout.nb_channels = 2;
        }
        else if (outChannelLayout.nb_channels > 1)
        {
            outChannelLayout.u.mask = AV_CH_LAYOUT_STEREO;
            outChannelLayout.nb_channels = 2;
        }
        else
        {
            outChannelLayout.u.mask = AV_CH_LAYOUT_MONO;
            outChannelLayout.nb_channels = 1;
        }
    }
    else if (nativeLayout)
    {
        // strip off advanced channels not supported by Windows APIs.
        if (nativeLayout > 0x000FFFFF)
        {
            nativeLayout &= 0x000FFFFF;
            if (!nativeLayout)
            {
                outChannelLayout.order = AV_CHANNEL_ORDER_UNSPEC;
            }
            else
            {
                outChannelLayout.u.mask = nativeLayout;
                outChannelLayout.nb_channels = std::_Popcount(nativeLayout);
            }
        }
    }

    needsUpdateResampler = true;
    bytesPerSample = av_get_bytes_per_sample(outSampleFormat);
    int bitsPerSample = bytesPerSample * 8;
    UINT32 reportedChannelLayout =
        nativeLayout == AV_CH_LAYOUT_STEREO_DOWNMIX
        ? AV_CH_LAYOUT_STEREO
        : (UINT32)nativeLayout;

    // set encoding properties
    auto encodingProperties = winrt::Windows::Media::MediaProperties::AudioEncodingProperties();
    encodingProperties.Subtype(outSampleFormat == AV_SAMPLE_FMT_FLT ? MediaEncodingSubtypes::Float() : MediaEncodingSubtypes::Pcm());
    encodingProperties.BitsPerSample(bitsPerSample);
    encodingProperties.SampleRate(outSampleRate);
    encodingProperties.ChannelCount(outChannelLayout.nb_channels);
    encodingProperties.Bitrate(bitsPerSample * outSampleRate * outChannelLayout.nb_channels);
    encodingProperties.Properties().Insert(MF_MT_AUDIO_CHANNEL_MASK, winrt::box_value(reportedChannelLayout));

    return winrt::Windows::Media::Core::AudioStreamDescriptor(encodingProperties);
}

HRESULT UncompressedAudioSampleProvider::CheckFormatChanged(AVSampleFormat format, AVChannelLayout* channelLayout, int sampleRate)
{
    HRESULT hr = S_OK;

    bool hasFormatChanged = format != inSampleFormat || av_channel_layout_compare(&inChannelLayout, channelLayout) || sampleRate != inSampleRate;
    if (hasFormatChanged)
    {
        inSampleFormat = format;
        inChannelLayout = *channelLayout;
        inSampleRate = outSampleRate = sampleRate;

        if (inSampleFormat != outSampleFormat || av_channel_layout_compare(&inChannelLayout, channelLayout) || inSampleRate != outSampleRate)
        {
            // set flag to update resampler on next frame
            needsUpdateResampler = true;
        }
    }

    return hr;
}

HRESULT UncompressedAudioSampleProvider::UpdateResampler()
{
    HRESULT hr = S_OK;

    useResampler = av_channel_layout_compare(&inChannelLayout, &outChannelLayout) || inSampleRate != outSampleRate || inSampleFormat != outSampleFormat;
    if (useResampler)
    {
        // Set up resampler to convert to output format and channel layout.
        hr = swr_alloc_set_opts2(
            &m_pSwrCtx,
            &outChannelLayout,
            outSampleFormat,
            outSampleRate,
            &inChannelLayout,
            inSampleFormat,
            inSampleRate,
            0,
            NULL);

        if (SUCCEEDED(hr))
        {
            if (swr_init(m_pSwrCtx) < 0)
            {
                hr = E_FAIL;
                useResampler = false;
            }
        }
    }

    // force update next time if there was an error
    needsUpdateResampler = FAILED(hr);

    return hr;
}

UncompressedAudioSampleProvider::~UncompressedAudioSampleProvider()
{
    // Free 
    swr_free(&m_pSwrCtx);
}

HRESULT UncompressedAudioSampleProvider::CreateBufferFromFrame(IBuffer* pBuffer, IDirect3DSurface* surface, AVFrame* avFrame, int64_t& framePts, int64_t& frameDuration)
{
    UNREFERENCED_PARAMETER(surface);

    HRESULT hr = S_OK;

    hr = CheckFormatChanged((AVSampleFormat)avFrame->format, &avFrame->ch_layout, avFrame->sample_rate);

    if (SUCCEEDED(hr) && needsUpdateResampler)
    {
        UpdateResampler();
    }

    if (SUCCEEDED(hr))
    {
        if (useResampler)
        {
            // Resample uncompressed frame to output format
            uint8_t** resampledData = nullptr;
            unsigned int aBufferSize = av_samples_alloc_array_and_samples(&resampledData, NULL, outChannelLayout.nb_channels, avFrame->nb_samples, outSampleFormat, 0);
            int resampledDataSize = swr_convert(m_pSwrCtx, resampledData, aBufferSize, (const uint8_t**)avFrame->extended_data, avFrame->nb_samples);

            if (resampledDataSize < 0)
            {
                hr = E_FAIL;
            }
            else
            {
                auto size = min(aBufferSize, (unsigned int)(resampledDataSize * outChannelLayout.nb_channels * bytesPerSample));
                *pBuffer = NativeBuffer::NativeBufferFactory::CreateNativeBuffer(resampledData[0], size, free_resample_buffer, resampledData);
            }
        }
        else
        {
            // Using direct buffer: just create a buffer reference to hand out to MSS pipeline
            auto bufferRef = av_buffer_ref(avFrame->buf[0]);
            if (bufferRef)
            {
                auto size = min(bufferRef->size, (size_t)avFrame->nb_samples * outChannelLayout.nb_channels * bytesPerSample);
                *pBuffer = NativeBuffer::NativeBufferFactory::CreateNativeBuffer(bufferRef->data, (UINT32)size, free_buffer, bufferRef);
            }
            else
            {
                hr = E_FAIL;
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        // Try to get the best effort timestamp for the frame.
        if (avFrame->best_effort_timestamp != AV_NOPTS_VALUE)
            framePts = avFrame->best_effort_timestamp;

        // always update duration with real decoded sample duration
        auto actualDuration = (long long)avFrame->nb_samples * m_pAvStream->time_base.den / ((long long)outSampleRate * m_pAvStream->time_base.num);
        if (actualDuration == 0)
        {
            actualDuration = 1;
        }

        if (frameDuration != actualDuration)
        {
            // compensate for start encoder padding (gapless playback)
            if (framePts == 0 && m_startOffset > 0 && (m_pAvStream->start_time + actualDuration) == frameDuration)
            {
                framePts = m_pAvStream->start_time;
            }
            frameDuration = actualDuration;
        }
    }

    return hr;
}

void UncompressedAudioSampleProvider::free_resample_buffer(void* ptr)
{
    uint8_t** resampledData = (uint8_t**)ptr;
    av_freep(&resampledData[0]); // free actual data
    av_free(resampledData);      // free array of pointers
}
