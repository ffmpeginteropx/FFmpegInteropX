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

using namespace FFmpegInteropX;

UncompressedAudioSampleProvider::UncompressedAudioSampleProvider(
    FFmpegReader^ reader,
    AVFormatContext* avFormatCtx,
    AVCodecContext* avCodecCtx,
    MediaSourceConfig^ config,
    int streamIndex)
    : UncompressedSampleProvider(reader, avFormatCtx, avCodecCtx, config, streamIndex, HardwareDecoderStatus::Unknown)
    , m_pSwrCtx(nullptr)
{
}

IMediaStreamDescriptor^ UncompressedAudioSampleProvider::CreateStreamDescriptor()
{
    frameProvider = ref new UncompressedFrameProvider(m_pAvFormatCtx, m_pAvCodecCtx, ref new AudioEffectFactory(m_pAvCodecCtx));

    auto format = m_pAvCodecCtx->sample_fmt != AV_SAMPLE_FMT_NONE ? m_pAvCodecCtx->sample_fmt : AV_SAMPLE_FMT_S16;
    auto channels = AvCodecContextHelpers::GetNBChannels(m_pAvCodecCtx);
    auto channelLayout = m_pAvCodecCtx->channel_layout ? m_pAvCodecCtx->channel_layout : AvCodecContextHelpers::GetDefaultChannelLayout(channels);
    auto sampleRate = m_pAvCodecCtx->sample_rate;

    inSampleFormat = format;
    inChannels = channels;
    inChannelLayout = channelLayout;
    inSampleRate = outSampleRate = sampleRate;

    outSampleFormat = av_get_packed_sample_fmt(inSampleFormat);

    // replace unsupported formats with closest format
    if (outSampleFormat == AV_SAMPLE_FMT_DBL)
        outSampleFormat = AV_SAMPLE_FMT_FLT;
    if (outSampleFormat == AV_SAMPLE_FMT_S64)
        outSampleFormat = AV_SAMPLE_FMT_S32;

    outChannels = inChannels;
    outChannelLayout = inChannelLayout;

    if (m_config->DownmixAudioStreamsToStereo && outChannelLayout > AV_CH_LAYOUT_STEREO)
    {
        // use existing downmix channels, if available, otherwise perform manual downmix using resampler
        if (outChannelLayout & AV_CH_LAYOUT_STEREO_DOWNMIX)
        {
            outChannelLayout = AV_CH_LAYOUT_STEREO_DOWNMIX;
            outChannels = 2;
        }
        else if (outChannels > 1)
        {
            outChannelLayout = AV_CH_LAYOUT_STEREO;
            outChannels = 2;
        }
        else
        {
            outChannelLayout = AV_CH_LAYOUT_MONO;
            outChannels = 1;
        }
    }
    else
    {
        // strip off advanced channels not supported by Windows APIs.
        if (outChannelLayout > 0x000FFFFF)
        {
            outChannelLayout &= 0x000FFFFF;
            if (!outChannelLayout)
            {
                outChannelLayout = AvCodecContextHelpers::GetDefaultChannelLayout(outChannels);
            }
            outChannels = av_get_channel_layout_nb_channels(outChannelLayout);
        }
    }

    needsUpdateResampler = true;
    bytesPerSample = av_get_bytes_per_sample(outSampleFormat);
    int bitsPerSample = bytesPerSample * 8;
    UINT32 reportedChannelLayout =
        outChannelLayout == AV_CH_LAYOUT_STEREO_DOWNMIX
        ? AV_CH_LAYOUT_STEREO
        : (UINT32)outChannelLayout;

    // set encoding properties
    auto encodingProperties = ref new AudioEncodingProperties();
    encodingProperties->Subtype = outSampleFormat == AV_SAMPLE_FMT_FLT ? MediaEncodingSubtypes::Float : MediaEncodingSubtypes::Pcm;
    encodingProperties->BitsPerSample = bitsPerSample;
    encodingProperties->SampleRate = outSampleRate;
    encodingProperties->ChannelCount = outChannels;
    encodingProperties->Bitrate = bitsPerSample * outSampleRate * outChannels;
    encodingProperties->Properties->Insert(MF_MT_AUDIO_CHANNEL_MASK, reportedChannelLayout);

    return ref new AudioStreamDescriptor(encodingProperties);
}

HRESULT UncompressedAudioSampleProvider::CheckFormatChanged(AVSampleFormat format, int channels, uint64 channelLayout, int sampleRate)
{
    HRESULT hr = S_OK;

    channelLayout = channelLayout ? channelLayout : AvCodecContextHelpers::GetDefaultChannelLayout(channels);
    bool hasFormatChanged = format != inSampleFormat || channels != inChannels || channelLayout != inChannelLayout || sampleRate != inSampleRate;
    if (hasFormatChanged)
    {
        inSampleFormat = format;
        inChannels = channels;
        inChannelLayout = channelLayout;
        inSampleRate = outSampleRate = sampleRate;

        if (inSampleFormat != outSampleFormat || inChannels != outChannels || inChannelLayout != outChannelLayout || inSampleRate != outSampleRate)
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

    useResampler = inChannels != outChannels || inChannelLayout != outChannelLayout || inSampleRate != outSampleRate || inSampleFormat != outSampleFormat;
    if (useResampler)
    {
        // Set up resampler to convert to output format and channel layout.
        m_pSwrCtx = swr_alloc_set_opts(
            m_pSwrCtx,
            outChannelLayout,
            outSampleFormat,
            outSampleRate,
            inChannelLayout,
            inSampleFormat,
            inSampleRate,
            0,
            NULL);

        if (!m_pSwrCtx)
        {
            hr = E_OUTOFMEMORY;
        }

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

HRESULT UncompressedAudioSampleProvider::CreateBufferFromFrame(IBuffer^* pBuffer, IDirect3DSurface^* surface, AVFrame* avFrame, int64_t& framePts, int64_t& frameDuration)
{
    HRESULT hr = S_OK;

    hr = CheckFormatChanged((AVSampleFormat)avFrame->format, avFrame->channels, avFrame->channel_layout, avFrame->sample_rate);

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
            unsigned int aBufferSize = av_samples_alloc_array_and_samples(&resampledData, NULL, outChannels, avFrame->nb_samples, outSampleFormat, 0);
            int resampledDataSize = swr_convert(m_pSwrCtx, resampledData, aBufferSize, (const uint8_t**)avFrame->extended_data, avFrame->nb_samples);

            if (resampledDataSize < 0)
            {
                hr = E_FAIL;
            }
            else
            {
                auto size = min(aBufferSize, (unsigned int)(resampledDataSize * outChannels * bytesPerSample));
                *pBuffer = NativeBuffer::NativeBufferFactory::CreateNativeBuffer(resampledData[0], size, av_freep, resampledData);
            }
        }
        else
        {
            // Using direct buffer: just create a buffer reference to hand out to MSS pipeline
            auto bufferRef = av_buffer_ref(avFrame->buf[0]);
            if (bufferRef)
            {
                auto size = min(bufferRef->size, avFrame->nb_samples * outChannels * bytesPerSample);
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
        // always update duration with real decoded sample duration
        auto actualDuration = (long long)avFrame->nb_samples * m_pAvStream->time_base.den / (outSampleRate * m_pAvStream->time_base.num);
        if (actualDuration == 0)
        {
            actualDuration = 1;
        }
        if (frameDuration != actualDuration)
        {
            // TODO check if this can be removed. start_skip_samples was made internal in ffmpeg 4.4...
            //// compensate for start encoder padding (gapless playback)
            //if (m_pAvStream->nb_decoded_frames == 1 && m_pAvStream->start_skip_samples > 0)
            //{
            //	// check if duration difference matches encoder padding
            //	auto skipDuration = (long long)m_pAvStream->start_skip_samples * m_pAvStream->time_base.den / (outSampleRate * m_pAvStream->time_base.num);
            //	if (skipDuration == frameDuration - actualDuration)
            //	{
            //		framePts += skipDuration;
            //	}
            //}
            frameDuration = actualDuration;
        }
    }

    return hr;
}

