#pragma once
#include "VideoStreamInfo.g.h"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropXWinUI::implementation
{
    struct VideoStreamInfo : VideoStreamInfoT<VideoStreamInfo>
    {
        VideoStreamInfo() = default;

        VideoStreamInfo(hstring const& name, hstring const& language, hstring const& codecName, FFmpegInteropXWinUI::StreamDisposition const& disposition, int64_t bitrate, bool isDefault, int32_t pixelWidth, int32_t pixelHeight, double displayAspectRatio, int32_t bitsPerSample, double framesPerSecond, FFmpegInteropXWinUI::HardwareDecoderStatus const& hwAccel, FFmpegInteropXWinUI::DecoderEngine const& decoderEngine);
        int32_t PixelWidth();
        int32_t PixelHeight();
        double DisplayAspectRatio();
        int32_t BitsPerSample();
        double FramesPerSecond();
        double FramesPerSecondOverride();
        void FramesPerSecondOverride(double value);
        FFmpegInteropXWinUI::HardwareDecoderStatus HardwareDecoderStatus();
        FFmpegInteropXWinUI::DecoderEngine DecoderEngine();
        hstring Name();
        hstring Language();
        hstring CodecName();
        FFmpegInteropXWinUI::StreamDisposition Disposition();
        int64_t Bitrate();
        bool IsDefault();
        FFmpegInteropXWinUI::DecoderEngine _DecoderEngine;

    public:
        hstring name;
        hstring language;
        hstring codecName;
        StreamDisposition disposition;
        int64_t bitrate;
        bool isDefault;

        int pixelWidth;
        int pixelHeight;
        double displayAspectRatio;
        int bitsPerSample;
        double framesPerSecond;

        FFmpegInteropXWinUI::HardwareDecoderStatus hardwareDecoderStatus;
        FFmpegInteropXWinUI::DecoderEngine decoderEngine;
    };
}
namespace winrt::FFmpegInteropXWinUI::factory_implementation
{
    struct VideoStreamInfo : VideoStreamInfoT<VideoStreamInfo, implementation::VideoStreamInfo>
    {
    };
}
