#include "pch.h"
#include "AudioStreamInfo.h"
#include "AudioStreamInfo.g.cpp"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropXWinUI::implementation
{
    AudioStreamInfo::AudioStreamInfo(hstring const& name, hstring const& language, hstring const& codecName, FFmpegInteropXWinUI::StreamDisposition const& disposition, int64_t bitrate, bool isDefault, int32_t channels, hstring const& channelLayout, int32_t sampleRate, int32_t bitsPerSample, FFmpegInteropXWinUI::DecoderEngine const& decoderEngine)
    {
        this->name = name;
        this->language = language;
        this->codecName = codecName;
        this->disposition = disposition;
        this->bitrate = bitrate;
        this->isDefault = isDefault;

        this->channels = channels;
        this->channelLayout = channelLayout;
        this->sampleRate = sampleRate;
        this->bitsPerSample = bitsPerSample;

        this->decoderEngine = decoderEngine;
    }

    int32_t AudioStreamInfo::Channels()
    {
        return channels;
    }
    
    hstring AudioStreamInfo::ChannelLayout()
    {
        return channelLayout;
    }
    
    int32_t AudioStreamInfo::SampleRate()
    {
        return sampleRate;
    }
    
    int32_t AudioStreamInfo::BitsPerSample()
    {
        return bitsPerSample;
    }
    
    FFmpegInteropXWinUI::DecoderEngine AudioStreamInfo::DecoderEngine()
    {
        return decoderEngine;
    }
    
    hstring AudioStreamInfo::Name()
    {
        return name;
    }
    
    hstring AudioStreamInfo::Language()
    {
        return language;
    }
    
    hstring AudioStreamInfo::CodecName()
    {
        return codecName;
    }
    
    FFmpegInteropXWinUI::StreamDisposition AudioStreamInfo::Disposition()
    {
        return disposition;
    }
    
    int64_t AudioStreamInfo::Bitrate()
    {
        return bitrate;
    }
    
    bool AudioStreamInfo::IsDefault()
    {
        return isDefault;
    }
}
