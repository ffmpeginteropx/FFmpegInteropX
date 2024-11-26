#pragma once
#include "AudioStreamInfo.g.h"

namespace winrt::FFmpegInteropX::implementation
{
    struct AudioStreamInfo : AudioStreamInfoT<AudioStreamInfo>
    {
        AudioStreamInfo() = default;

        AudioStreamInfo(hstring const& name, hstring const& language, hstring const& codecName, winrt::FFmpegInteropX::StreamDisposition const& disposition, int64_t bitrate, bool isDefault, int32_t channels, hstring const& channelLayout, int32_t sampleRate, int32_t bitsPerSample, winrt::FFmpegInteropX::DecoderEngine const& decoderEngine, int32_t streamIndex);
        int32_t Channels();
        hstring ChannelLayout();
        int32_t SampleRate();
        int32_t BitsPerSample();
        winrt::FFmpegInteropX::DecoderEngine DecoderEngine();
        hstring Name();
        hstring Language();
        hstring CodecName();
        winrt::FFmpegInteropX::StreamDisposition Disposition();
        int64_t Bitrate();
        bool IsDefault();
        int32_t StreamIndex();
        bool IsActive()
        {
            return active;
        }

    public:
        bool SetDefault()
        {
            isDefault = true;
            return isDefault;
        }

        bool SetIsActive(bool value)
        {
            active = value;
            return active;
        }

    private:
        hstring name{};
        hstring language{};
        hstring codecName{};
        StreamDisposition disposition;
        int64_t bitrate = 0;
        bool isDefault = false;

        int channels = 0;
        hstring channelLayout{};
        int sampleRate = 0;
        int bitsPerSample = 0;

        FFmpegInteropX::DecoderEngine decoderEngine;
        int streamIndex = -1;
        bool active = false;
    };
}
namespace winrt::FFmpegInteropX::factory_implementation
{
    struct AudioStreamInfo : AudioStreamInfoT<AudioStreamInfo, implementation::AudioStreamInfo>
    {
    };
}
