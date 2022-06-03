#include "pch.h"
#include "VideoStreamInfo.h"
#include "VideoStreamInfo.g.cpp"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropXWinUI::implementation
{
    VideoStreamInfo::VideoStreamInfo(hstring const& name, hstring const& language, hstring const& codecName, FFmpegInteropXWinUI::StreamDisposition const& disposition, int64_t bitrate, bool isDefault, int32_t pixelWidth, int32_t pixelHeight, double displayAspectRatio, int32_t bitsPerSample, double framesPerSecond, FFmpegInteropXWinUI::HardwareDecoderStatus const& hwAccel, FFmpegInteropXWinUI::DecoderEngine const& decoderEngine)
    {
        throw hresult_not_implemented();
    }
    int32_t VideoStreamInfo::PixelWidth()
    {
        throw hresult_not_implemented();
    }
    int32_t VideoStreamInfo::PixelHeight()
    {
        throw hresult_not_implemented();
    }
    double VideoStreamInfo::DisplayAspectRatio()
    {
        throw hresult_not_implemented();
    }
    int32_t VideoStreamInfo::BitsPerSample()
    {
        throw hresult_not_implemented();
    }
    double VideoStreamInfo::FramesPerSecond()
    {
        throw hresult_not_implemented();
    }
    double VideoStreamInfo::FramesPerSecondOverride()
    {
        throw hresult_not_implemented();
    }
    void VideoStreamInfo::FramesPerSecondOverride(double value)
    {
        throw hresult_not_implemented();
    }
    FFmpegInteropXWinUI::HardwareDecoderStatus VideoStreamInfo::HardwareDecoderStatus()
    {
        throw hresult_not_implemented();
    }
    FFmpegInteropXWinUI::DecoderEngine VideoStreamInfo::DecoderEngine()
    {
        throw hresult_not_implemented();
    }
    hstring VideoStreamInfo::Name()
    {
        throw hresult_not_implemented();
    }
    hstring VideoStreamInfo::Language()
    {
        throw hresult_not_implemented();
    }
    hstring VideoStreamInfo::CodecName()
    {
        throw hresult_not_implemented();
    }
    FFmpegInteropXWinUI::StreamDisposition VideoStreamInfo::Disposition()
    {
        throw hresult_not_implemented();
    }
    int64_t VideoStreamInfo::Bitrate()
    {
        throw hresult_not_implemented();
    }
    bool VideoStreamInfo::IsDefault()
    {
        throw hresult_not_implemented();
    }
}
