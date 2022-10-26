#pragma once
#include "VideoStreamInfo.g.h"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropX::implementation
{
    struct VideoStreamInfo : VideoStreamInfoT<VideoStreamInfo>
    {
        VideoStreamInfo(hstring const& name, hstring const& language, hstring const& codecName, FFmpegInteropX::StreamDisposition const& disposition, int64_t bitrate, bool isDefault, int32_t pixelWidth, int32_t pixelHeight, double displayAspectRatio, int32_t bitsPerSample, double framesPerSecond, FFmpegInteropX::HardwareDecoderStatus const& hwAccel, FFmpegInteropX::DecoderEngine const& decoderEngine);
        int32_t PixelWidth();
        int32_t PixelHeight();
        double DisplayAspectRatio();
        int32_t BitsPerSample();
        double FramesPerSecond();

        ///<summary>Override the frame rate of the video stream.</summary>
        ///<remarks>
        /// This must be set before calling CreatePlaybackItem().
        /// Setting this can cause A/V desync, since it will only affect this stream.
        /// </remarks>
        double FramesPerSecondOverride();
        void FramesPerSecondOverride(double value);
        FFmpegInteropX::HardwareDecoderStatus HardwareDecoderStatus();
        FFmpegInteropX::DecoderEngine DecoderEngine();
        hstring Name();
        hstring Language();
        hstring CodecName();
        FFmpegInteropX::StreamDisposition Disposition();
        int64_t Bitrate();
        bool IsDefault();
        FFmpegInteropX::DecoderEngine _DecoderEngine;

    public:
        hstring name{};
        hstring language{};
        hstring codecName{};
        StreamDisposition disposition;
        int64_t bitrate = 0;
        bool isDefault = false;

        int pixelWidth = 0;
        int pixelHeight = 0;
        double displayAspectRatio = 0.0;
        int bitsPerSample = 0;
        double framesPerSecond = 0;
        double framesPerSecondOverride = 0;
        FFmpegInteropX::HardwareDecoderStatus hardwareDecoderStatus;
        FFmpegInteropX::DecoderEngine decoderEngine;

        void SetDefault()
        {
            isDefault = true;
        }
    };
}
namespace winrt::FFmpegInteropX::factory_implementation
{
    struct VideoStreamInfo : VideoStreamInfoT<VideoStreamInfo, implementation::VideoStreamInfo>
    {
    };
}
