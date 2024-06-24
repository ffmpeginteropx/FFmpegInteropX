#pragma once
#include "VideoStreamInfo.g.h"

namespace winrt::FFmpegInteropX::implementation
{
    struct VideoStreamInfo : VideoStreamInfoT<VideoStreamInfo>
    {
        VideoStreamInfo(hstring const& name, hstring const& language, hstring const& codecName, winrt::FFmpegInteropX::StreamDisposition const& disposition, int64_t bitrate, bool isDefault, int32_t pixelWidth, int32_t pixelHeight, double displayAspectRatio, int32_t bitsPerSample, double framesPerSecond, winrt::FFmpegInteropX::HardwareDecoderStatus const& hwAccel, winrt::FFmpegInteropX::DecoderEngine const& decoderEngine, int32_t streamIndex);
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
        void FramesPerSecondOverride(double value);
        double FramesPerSecondOverride();

        winrt::FFmpegInteropX::HardwareDecoderStatus HardwareDecoderStatus();
        winrt::FFmpegInteropX::DecoderEngine DecoderEngine();
        hstring Name();
        hstring Language();
        hstring CodecName();
        winrt::FFmpegInteropX::StreamDisposition Disposition();
        int64_t Bitrate();
        bool IsDefault();
        int32_t StreamIndex();

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
        int streamIndex = -1;
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
