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
        throw hresult_not_implemented();
    }
    int32_t AudioStreamInfo::Channels()
    {
        throw hresult_not_implemented();
    }
    hstring AudioStreamInfo::ChannelLayout()
    {
        throw hresult_not_implemented();
    }
    int32_t AudioStreamInfo::SampleRate()
    {
        throw hresult_not_implemented();
    }
    int32_t AudioStreamInfo::BitsPerSample()
    {
        throw hresult_not_implemented();
    }
    FFmpegInteropXWinUI::DecoderEngine AudioStreamInfo::DecoderEngine()
    {
        throw hresult_not_implemented();
    }
    hstring AudioStreamInfo::Name()
    {
        throw hresult_not_implemented();
    }
    hstring AudioStreamInfo::Language()
    {
        throw hresult_not_implemented();
    }
    hstring AudioStreamInfo::CodecName()
    {
        throw hresult_not_implemented();
    }
    FFmpegInteropXWinUI::StreamDisposition AudioStreamInfo::Disposition()
    {
        throw hresult_not_implemented();
    }
    int64_t AudioStreamInfo::Bitrate()
    {
        throw hresult_not_implemented();
    }
    bool AudioStreamInfo::IsDefault()
    {
        throw hresult_not_implemented();
    }
}
