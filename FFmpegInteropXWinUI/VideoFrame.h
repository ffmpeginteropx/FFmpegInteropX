#pragma once
#include "VideoFrame.g.h"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropXWinUI::implementation
{
    struct VideoFrame : VideoFrameT<VideoFrame>
    {
        VideoFrame() = default;

        VideoFrame(Windows::Storage::Streams::IBuffer const& pixelData, uint32_t width, uint32_t height, Windows::Media::MediaProperties::MediaRatio const& pixelAspectRatio, Windows::Foundation::TimeSpan const& timestamp);
        Windows::Storage::Streams::IBuffer PixelData();
        uint32_t PixelWidth();
        uint32_t PixelHeight();
        Windows::Media::MediaProperties::MediaRatio PixelAspectRatio();
        Windows::Foundation::TimeSpan Timestamp();
        Windows::Foundation::IAsyncAction EncodeAsBmpAsync(Windows::Storage::Streams::IRandomAccessStream stream);
        Windows::Foundation::IAsyncAction EncodeAsJpegAsync(Windows::Storage::Streams::IRandomAccessStream stream);
        Windows::Foundation::IAsyncAction EncodeAsPngAsync(Windows::Storage::Streams::IRandomAccessStream stream);
        uint32_t DisplayWidth();
        uint32_t DisplayHeight();
        double DisplayAspectRatio();
    };
}
namespace winrt::FFmpegInteropXWinUI::factory_implementation
{
    struct VideoFrame : VideoFrameT<VideoFrame, implementation::VideoFrame>
    {
    };
}
