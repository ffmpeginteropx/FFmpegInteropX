#pragma once
#include "AudioStreamInfo.g.h"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropX::implementation
{
    struct AudioStreamInfo : AudioStreamInfoT<AudioStreamInfo>
    {
        AudioStreamInfo() = default;

        AudioStreamInfo(hstring const& name, hstring const& language, hstring const& codecName, FFmpegInteropX::StreamDisposition const& disposition, int64_t bitrate, bool isDefault, int32_t channels, hstring const& channelLayout, int32_t sampleRate, int32_t bitsPerSample, FFmpegInteropX::DecoderEngine const& decoderEngine);
        int32_t Channels();
        hstring ChannelLayout();
        int32_t SampleRate();
        int32_t BitsPerSample();
        FFmpegInteropX::DecoderEngine DecoderEngine();
        hstring Name();
        hstring Language();
        hstring CodecName();
        FFmpegInteropX::StreamDisposition Disposition();
        int64_t Bitrate();
    public:
        bool IsDefault();
        bool SetDefault()
        {
            isDefault = true;
            return isDefault;
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
    };
}
namespace winrt::FFmpegInteropX::factory_implementation
{
    struct AudioStreamInfo : AudioStreamInfoT<AudioStreamInfo, implementation::AudioStreamInfo>
    {
    };
}
