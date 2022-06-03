#include "pch.h"
#include "VideoFrame.h"
#include "VideoFrame.g.cpp"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropXWinUI::implementation
{
    VideoFrame::VideoFrame(Windows::Storage::Streams::IBuffer const& pixelData, uint32_t width, uint32_t height, Windows::Media::MediaProperties::MediaRatio const& pixelAspectRatio, Windows::Foundation::TimeSpan const& timestamp)
    {
        throw hresult_not_implemented();
    }
    Windows::Storage::Streams::IBuffer VideoFrame::PixelData()
    {
        throw hresult_not_implemented();
    }
    uint32_t VideoFrame::PixelWidth()
    {
        throw hresult_not_implemented();
    }
    uint32_t VideoFrame::PixelHeight()
    {
        throw hresult_not_implemented();
    }
    Windows::Media::MediaProperties::MediaRatio VideoFrame::PixelAspectRatio()
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::TimeSpan VideoFrame::Timestamp()
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::IAsyncAction VideoFrame::EncodeAsBmpAsync(Windows::Storage::Streams::IRandomAccessStream stream)
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::IAsyncAction VideoFrame::EncodeAsJpegAsync(Windows::Storage::Streams::IRandomAccessStream stream)
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::IAsyncAction VideoFrame::EncodeAsPngAsync(Windows::Storage::Streams::IRandomAccessStream stream)
    {
        throw hresult_not_implemented();
    }
    uint32_t VideoFrame::DisplayWidth()
    {
        throw hresult_not_implemented();
    }
    uint32_t VideoFrame::DisplayHeight()
    {
        throw hresult_not_implemented();
    }
    double VideoFrame::DisplayAspectRatio()
    {
        throw hresult_not_implemented();
    }
}
