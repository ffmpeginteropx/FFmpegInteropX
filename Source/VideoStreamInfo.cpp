#include "pch.h"
#include "VideoStreamInfo.h"
#include "VideoStreamInfo.g.cpp"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropX::implementation
{
    VideoStreamInfo::VideoStreamInfo(hstring const& name, hstring const& language, hstring const& codecName, FFmpegInteropX::StreamDisposition const& disposition, int64_t bitrate, bool isDefault, int32_t pixelWidth, int32_t pixelHeight, double displayAspectRatio, int32_t bitsPerSample, double framesPerSecond, FFmpegInteropX::HardwareDecoderStatus const& hwAccel, FFmpegInteropX::DecoderEngine const& decoderEngine)
    {
        this->name = name;
        this->language = language;
        this->codecName = codecName;
        this->disposition = disposition;
        this->bitrate = bitrate;
        this->isDefault = isDefault;

        this->pixelWidth = pixelWidth;
        this->pixelHeight = pixelHeight;
        this->displayAspectRatio = displayAspectRatio;
        this->bitsPerSample = bitsPerSample;
        this->framesPerSecond = framesPerSecond;
        this->hardwareDecoderStatus = hwAccel;
        this->decoderEngine = decoderEngine;
    }
    int32_t VideoStreamInfo::PixelWidth()
    {
        return pixelWidth;
    }
    int32_t VideoStreamInfo::PixelHeight()
    {
        return pixelHeight;
    }
    double VideoStreamInfo::DisplayAspectRatio()
    {
        return displayAspectRatio;
    }
    int32_t VideoStreamInfo::BitsPerSample()
    {
        return bitsPerSample;
    }
    double VideoStreamInfo::FramesPerSecond()
    {
        return framesPerSecond;
    }
    double VideoStreamInfo::FramesPerSecondOverride()
    {
        return framesPerSecondOverride;
    }
    void VideoStreamInfo::FramesPerSecondOverride(double value)
    {
        framesPerSecondOverride = value;
    }
    FFmpegInteropX::HardwareDecoderStatus VideoStreamInfo::HardwareDecoderStatus()
    {
        return hardwareDecoderStatus;
    }
    FFmpegInteropX::DecoderEngine VideoStreamInfo::DecoderEngine()
    {
        return decoderEngine;
    }
    hstring VideoStreamInfo::Name()
    {
        return name;
    }
    hstring VideoStreamInfo::Language()
    {
        return language;
    }
    hstring VideoStreamInfo::CodecName()
    {
        return codecName;
    }
    FFmpegInteropX::StreamDisposition VideoStreamInfo::Disposition()
    {
        return disposition;
    }
    int64_t VideoStreamInfo::Bitrate()
    {
        return bitrate;
    }
    bool VideoStreamInfo::IsDefault()
    {
        return isDefault;
    }
}
