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
#include "MediaSampleProvider.h"
#include "FFmpegMediaSource.h"
#include "FFmpegReader.h"
#include "LanguageTagConverter.h"
#include "AvCodecContextHelpers.h"
#include "Mfapi.h"

using namespace winrt::Windows::Media::MediaProperties;
using namespace winrt::Windows::Globalization;
using namespace winrt::Windows::Media::Core;
using namespace winrt::Windows::Graphics::DirectX::Direct3D11;
using namespace winrt::Windows::Storage::Streams;

MediaSampleProvider::MediaSampleProvider(
    std::shared_ptr<FFmpegReader> reader,
    AVFormatContext* avFormatCtx,
    AVCodecContext* avCodecCtx,
    winrt::FFmpegInteropX::MediaSourceConfig const& config,
    int streamIndex,
    HardwareDecoderStatus hardwareDecoderStatus)
    : m_pReader(reader)
    , m_pAvFormatCtx(avFormatCtx)
    , m_pAvCodecCtx(avCodecCtx)
    , m_pAvStream(avFormatCtx->streams[streamIndex])
    , m_config(config)
    , m_streamIndex(streamIndex)
    , hardwareDecoderStatus(hardwareDecoderStatus)
{
    DebugMessage(L"MediaSampleProvider\n");

    timeBaseFactor = av_q2d(m_pAvStream->time_base) * 10000000;

    // init first packet pts time from start_time
    if (m_pAvFormatCtx->streams[m_streamIndex]->start_time == AV_NOPTS_VALUE)
    {
        //if start time is AV_NOPTS_VALUE, set it to 0
        m_nextPacketPts = 0;
    }
    else
    {
        //otherwise set the start time of the first packet to the stream start time.
        m_nextPacketPts = m_pAvFormatCtx->streams[m_streamIndex]->start_time;
    }
}

MediaSampleProvider::~MediaSampleProvider()
{
    DebugMessage(L"~MediaSampleProvider\n");

    avcodec_free_context(&m_pAvCodecCtx);

    Flush();
    device = nullptr;
    deviceContext = nullptr;
}

HRESULT MediaSampleProvider::Initialize()
{
    m_streamDescriptor = CreateStreamDescriptor();
    if (m_streamDescriptor)
    {
        InitializeNameLanguageCodec();
    }
    InitializeStreamInfo();
    return m_streamDescriptor ? S_OK : E_FAIL;
}

void MediaSampleProvider::InitializeNameLanguageCodec()
{
    // unfortunately, setting Name or Language on MediaStreamDescriptor does not have any effect, they are not shown in track selection list
    auto title = av_dict_get(m_pAvStream->metadata, "title", NULL, 0);
    if (title)
    {
        Name = StringUtils::Utf8ToPlatformString(title->value);
    }

    auto language = av_dict_get(m_pAvStream->metadata, "language", NULL, 0);
    if (language)
    {
        Language = StringUtils::Utf8ToPlatformString(language->value);
        if (Language.size() == 3)
        {
            auto entry = LanguageTagConverter::TryGetLanguage(Language);
            if (entry != nullptr)
            {
                try
                {
                    auto winLanguage = winrt::Windows::Globalization::Language(entry->TwoLetterCode());
                    Language = winLanguage.DisplayName();
                }
                catch (...)
                {
                    Language = entry->EnglishName();
                }
            }
        }
        else if (Language.size() == 2)
        {
            try
            {
                auto winLanguage = winrt::Windows::Globalization::Language(Language);
                Language = winLanguage.DisplayName();
            }
            catch (...)
            {
            }
        }

        if (Name.empty())
        {
            Name = Language;
        }
    }

    if (Name.empty() && (m_pAvStream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO || m_pAvStream->codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE))
    {
        int count = 0;
        int number = 0;
        for (int i = 0; i < (int)m_pAvFormatCtx->nb_streams; i++)
        {
            if (m_pAvFormatCtx->streams[i]->codecpar->codec_type == m_pAvStream->codecpar->codec_type)
            {
                count++;
                if (i == StreamIndex())
                {
                    number = count;
                }
            }
        }

        winrt::hstring name = m_pAvStream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO ? m_config.DefaultAudioStreamName() : m_config.DefaultSubtitleStreamName();
        if (count > 1)
        {
            name = name + L" " + to_wstring(number);
        }
        Name = name;
    }

    auto codec = m_pAvCodecCtx->codec_descriptor->name;
    if (codec)
    {
        CodecName = StringUtils::Utf8ToPlatformString(codec);
    }
}

void MediaSampleProvider::InitializeStreamInfo()
{
    switch (m_pAvCodecCtx->codec_type)
    {
    case AVMEDIA_TYPE_AUDIO:
    {
        auto channels = AvCodecContextHelpers::GetNBChannels(m_pAvCodecCtx);
        auto bitsPerSample = max(m_pAvStream->codecpar->bits_per_raw_sample, m_pAvStream->codecpar->bits_per_coded_sample);

        winrt::hstring channelLayout = L"";
        char* channelLayoutName = new char[256];
        if (channelLayoutName)
        {
            auto layout = m_pAvCodecCtx->channel_layout ? m_pAvCodecCtx->channel_layout : AvCodecContextHelpers::GetDefaultChannelLayout(channels);
            av_get_channel_layout_string(channelLayoutName, 256, channels, layout);
            channelLayout = StringUtils::Utf8ToPlatformString(channelLayoutName);
            delete[] channelLayoutName;
        }

        streamInfo = AudioStreamInfo(
            Name, Language, CodecName, (StreamDisposition)m_pAvStream->disposition, m_pAvStream->codecpar->bit_rate, false,
            channels, channelLayout, m_pAvStream->codecpar->sample_rate, bitsPerSample, Decoder());

        break;
    }
    case AVMEDIA_TYPE_VIDEO:
    {
        auto streamDescriptor = StreamDescriptor().as<VideoStreamDescriptor>();
        auto pixelAspect = (double)streamDescriptor.EncodingProperties().PixelAspectRatio().Numerator() / streamDescriptor.EncodingProperties().PixelAspectRatio().Denominator();
        auto videoAspect = ((double)m_pAvCodecCtx->width / m_pAvCodecCtx->height) / pixelAspect;
        auto bitsPerSample = max(m_pAvStream->codecpar->bits_per_raw_sample, m_pAvStream->codecpar->bits_per_coded_sample);
        auto framesPerSecond = m_pAvStream->avg_frame_rate.num > 0 && m_pAvStream->avg_frame_rate.den > 0 ? av_q2d(m_pAvStream->avg_frame_rate) : 0.0;

        streamInfo = VideoStreamInfo(Name, Language, CodecName, (StreamDisposition)m_pAvStream->disposition, m_pAvStream->codecpar->bit_rate, false,
            m_pAvStream->codecpar->width, m_pAvStream->codecpar->height, videoAspect,
            bitsPerSample, framesPerSecond, HardwareAccelerationStatus(), Decoder());

        break;
    }
    }
}

MediaStreamSample MediaSampleProvider::GetNextSample()
{
    DebugMessage(L"GetNextSample\n");

    HRESULT hr = S_OK;

    MediaStreamSample sample = { nullptr };
    if (m_isEnabled)
    {
        IBuffer buffer = nullptr;
        LONGLONG pts = 0;
        LONGLONG dur = 0;
        IDirect3DSurface surface;
        hr = CreateNextSampleBuffer(&buffer, pts, dur, &surface);

        if (hr == S_OK)
        {
            TimeSpan position = ConvertPosition(pts);
            TimeSpan duration = ConvertDuration(dur);

            if (surface)
            {
                sample = MediaStreamSample::CreateFromDirect3D11Surface(surface, position);
            }
            else
            {
                sample = MediaStreamSample::CreateFromBuffer(buffer, position);
            }
            sample.Duration(duration);
            sample.Discontinuous(m_isDiscontinuous);

            LastSampleTimestamp = position;

            hr = SetSampleProperties(sample);

            m_isDiscontinuous = false;
        }
        else if (hr == S_FALSE)
        {
            DebugMessage(L"End of stream reached.\n");
            DisableStream();
        }
        else
        {
            DebugMessage(L"Error reading next packet.\n");
            DisableStream();
        }
    }

    return sample;
}

HRESULT MediaSampleProvider::GetNextPacket(AVPacket** avPacket, LONGLONG& packetPts, LONGLONG& packetDuration)
{
    HRESULT hr = S_OK;
    unsigned int errorCount = 0;

    // Continue reading until there is an appropriate packet in the stream
    while (m_packetQueue.empty())
    {
        auto result = m_pReader->ReadPacket();
        if (result < 0)
        {
            if (result == AVERROR_EOF || (m_pAvFormatCtx->pb && m_pAvFormatCtx->pb->eof_reached))
            {
                DebugMessage(L"GetNextPacket reaching EOF\n");
                break;
            }
            else if (errorCount++ >= m_config.SkipErrors())
            {
                DebugMessage(L"Aborting after to too many read errors.\n");
                break;
            }
            else
            {
                DebugMessage(L"Read error.\n");
            }
        }
    }

    if (!m_packetQueue.empty())
    {
        // read next packet and set pts values
        auto packet = PopPacket();
        *avPacket = packet;

        packetDuration = packet->duration;

        if (packet->pts != AV_NOPTS_VALUE)
        {
            packetPts = packet->pts;
            // Set the PTS for the next sample if it doesn't one.
            m_nextPacketPts = packetPts + packetDuration;
        }
        else if (m_isDiscontinuous && packet->dts != AV_NOPTS_VALUE)
        {
            packetPts = packet->dts;
            // Use DTS instead of PTS after a seek, if PTS is not available (e.g. some WMV files)
            m_nextPacketPts = packetPts + packetDuration;
        }
        else
        {
            packetPts = m_nextPacketPts;
            // Set the PTS for the next sample if it doesn't one.
            m_nextPacketPts += packetDuration;
        }
    }
    else
    {
        hr = S_FALSE;
    }

    return hr;
}

HRESULT MediaSampleProvider::GetNextPacketTimestamp(TimeSpan& timestamp, TimeSpan& packetDuration)
{
    HRESULT hr = S_FALSE;

    // Continue reading until there is an appropriate packet in the stream
    while (m_packetQueue.empty())
    {
        if (m_pReader->ReadPacket() < 0)
        {
            DebugMessage(L"GetNextPacketTimestamp reaching EOF\n");
            break;
        }
    }

    if (!m_packetQueue.empty())
    {
        // peek next packet and set pts value
        auto packet = m_packetQueue.front();
        auto pts = packet->pts != AV_NOPTS_VALUE ? packet->pts : packet->dts;
        if (pts != AV_NOPTS_VALUE)
        {
            timestamp = ConvertPosition(pts);
            packetDuration = ConvertDuration(packet->duration);
            hr = S_OK;
        }
    }

    return hr;
}

HRESULT MediaSampleProvider::SkipPacketsUntilTimestamp(TimeSpan const& timestamp)
{
    HRESULT hr = S_OK;
    bool foundPacket = false;

    while (hr == S_OK && !foundPacket)
    {
        // Continue reading until there is an appropriate packet in the stream
        while (m_packetQueue.empty())
        {
            if (m_pReader->ReadPacket() < 0)
            {
                DebugMessage(L"SkipPacketsUntilTimestamp reaching EOF\n");
                break;
            }
        }

        if (!m_packetQueue.empty())
        {
            // peek next packet and check pts value
            auto packet = m_packetQueue.front();

            auto pts = packet->pts != AV_NOPTS_VALUE ? packet->pts : packet->dts;
            if (pts != AV_NOPTS_VALUE && packet->duration != AV_NOPTS_VALUE)
            {
                auto packetEnd = ConvertPosition(pts + packet->duration);
                if (packet->duration > 0 ? packetEnd <= timestamp : packetEnd < timestamp)
                {
                    m_packetQueue.pop();
                    av_packet_free(&packet);
                }
                else
                {
                    foundPacket = true;
                    break;
                }
            }
            else
            {
                hr = S_FALSE;
                break;
            }
        }
        else
        {
            // no more packet found
            hr = S_FALSE;
        }
    }

    return hr;
}

void MediaSampleProvider::QueuePacket(AVPacket* packet)
{
    DebugMessage(L" - QueuePacket\n");

    if (m_isEnabled)
    {
        m_packetQueue.push(packet);
    }
    else
    {
        av_packet_free(&packet);
    }
}

AVPacket* MediaSampleProvider::PopPacket()
{
    DebugMessage(L" - PopPacket\n");
    AVPacket* result = NULL;

    if (!m_packetQueue.empty())
    {
        result = m_packetQueue.front();
        m_packetQueue.pop();
    }

    return result;
}

void MediaSampleProvider::Flush()
{
    DebugMessage(L"Flush\n");
    while (!m_packetQueue.empty())
    {
        AVPacket* avPacket = PopPacket();
        av_packet_free(&avPacket);
    }
    if (m_pAvCodecCtx)
    {
        avcodec_flush_buffers(m_pAvCodecCtx);
    }
    m_isDiscontinuous = true;
    IsCleanSample = false;
}

void MediaSampleProvider::EnableStream()
{
    DebugMessage(L"EnableStream\n");
    m_isEnabled = true;
    m_pAvStream->discard = AVDISCARD_DEFAULT;
}

void MediaSampleProvider::DisableStream()
{
    DebugMessage(L"DisableStream\n");
    Flush();
    m_isEnabled = false;
    m_pAvStream->discard = AVDISCARD_ALL;
}

void MediaSampleProvider::SetCommonVideoEncodingProperties(VideoEncodingProperties const& videoProperties, bool isCompressedFormat)
{
    if (isCompressedFormat)
    {
        videoProperties.Width(m_pAvCodecCtx->width);
        videoProperties.Height(m_pAvCodecCtx->height);
        videoProperties.ProfileId(m_pAvCodecCtx->profile);
        if (m_pAvCodecCtx->bit_rate > 0)
        {
            videoProperties.Bitrate((unsigned int)m_pAvCodecCtx->bit_rate);
        }
    }

    if (m_pAvCodecCtx->sample_aspect_ratio.num > 0 &&
        m_pAvCodecCtx->sample_aspect_ratio.den > 0 &&
        m_pAvCodecCtx->sample_aspect_ratio.num != m_pAvCodecCtx->sample_aspect_ratio.den)
    {
        videoProperties.PixelAspectRatio().Numerator(m_pAvCodecCtx->sample_aspect_ratio.num);
        videoProperties.PixelAspectRatio().Denominator(m_pAvCodecCtx->sample_aspect_ratio.den);
    }
    else
    {
        videoProperties.PixelAspectRatio().Numerator(1);
        videoProperties.PixelAspectRatio().Denominator(1);
    }

    // set video rotation
    bool rotateVideo = false;
    int rotationAngle = 0;
    AVDictionaryEntry* rotate_tag = av_dict_get(m_pAvStream->metadata, "rotate", NULL, 0);
    if (rotate_tag != NULL)
    {
        rotateVideo = true;
        rotationAngle = atoi(rotate_tag->value);
    }
    else
    {
        rotateVideo = false;
    }
    if (rotateVideo)
    {
        videoProperties.Properties().Insert(MF_MT_VIDEO_ROTATION, winrt::box_value((UINT32)rotationAngle));
    }

    // Detect the correct framerate
    if (m_pAvCodecCtx->framerate.num != 0 || m_pAvCodecCtx->framerate.den != 1)
    {
        videoProperties.FrameRate().Numerator(m_pAvCodecCtx->framerate.num);
        videoProperties.FrameRate().Denominator(m_pAvCodecCtx->framerate.den);
    }
    else if (m_pAvStream->avg_frame_rate.num != 0 || m_pAvStream->avg_frame_rate.den != 0)
    {
        videoProperties.FrameRate().Numerator(m_pAvStream->avg_frame_rate.num);
        videoProperties.FrameRate().Denominator(m_pAvStream->avg_frame_rate.den);
    }
}

void MediaSampleProvider::Detach()
{
    Flush();
    m_pReader = nullptr;
    avcodec_free_context(&m_pAvCodecCtx);
}

void free_buffer(void* lpVoid)
{
    auto buffer = (AVBufferRef*)lpVoid;
    av_buffer_unref(&buffer);
}
