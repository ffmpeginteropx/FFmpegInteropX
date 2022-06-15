#pragma once
#include "AudioStreamInfo.g.h"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropXWinUI::implementation
{
    struct AudioStreamInfo : AudioStreamInfoT<AudioStreamInfo>
    {
        AudioStreamInfo() = default;

        AudioStreamInfo(hstring const& name, hstring const& language, hstring const& codecName, FFmpegInteropXWinUI::StreamDisposition const& disposition, int64_t bitrate, bool isDefault, int32_t channels, hstring const& channelLayout, int32_t sampleRate, int32_t bitsPerSample, FFmpegInteropXWinUI::DecoderEngine const& decoderEngine);
        int32_t Channels();
        hstring ChannelLayout();
        int32_t SampleRate();
        int32_t BitsPerSample();
        FFmpegInteropXWinUI::DecoderEngine DecoderEngine();
        hstring Name();
        hstring Language();
        hstring CodecName();
        FFmpegInteropXWinUI::StreamDisposition Disposition();
        int64_t Bitrate();
    public:
        bool IsDefault();
        bool SetDefault()
        {
            isDefault = true;
            return isDefault;
        }

    private:
        hstring name;
        hstring language;
        hstring codecName;
        StreamDisposition disposition;
        int64_t bitrate;
        bool isDefault;

        int channels;
        hstring channelLayout;
        int sampleRate;
        int bitsPerSample;

        FFmpegInteropXWinUI::DecoderEngine decoderEngine;
    };
}
namespace winrt::FFmpegInteropXWinUI::factory_implementation
{
    struct AudioStreamInfo : AudioStreamInfoT<AudioStreamInfo, implementation::AudioStreamInfo>
    {
    };
}
