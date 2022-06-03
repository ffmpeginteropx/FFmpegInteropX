#include "pch.h"
#include "FrameGrabber.h"
#include "FrameGrabber.g.cpp"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropXWinUI::implementation
{
    Windows::Foundation::IAsyncOperation<FFmpegInteropXWinUI::FrameGrabber> FrameGrabber::CreateFromStreamAsync(Windows::Storage::Streams::IRandomAccessStream stream)
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::IAsyncOperation<FFmpegInteropXWinUI::FrameGrabber> FrameGrabber::CreateFromUriAsync(hstring uri)
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::TimeSpan FrameGrabber::Duration()
    {
        throw hresult_not_implemented();
    }
    int32_t FrameGrabber::DecodePixelWidth()
    {
        throw hresult_not_implemented();
    }
    void FrameGrabber::DecodePixelWidth(int32_t value)
    {
        throw hresult_not_implemented();
    }
    int32_t FrameGrabber::DecodePixelHeight()
    {
        throw hresult_not_implemented();
    }
    void FrameGrabber::DecodePixelHeight(int32_t value)
    {
        throw hresult_not_implemented();
    }
    FFmpegInteropXWinUI::VideoStreamInfo FrameGrabber::CurrentVideoStream()
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::IAsyncOperation<FFmpegInteropXWinUI::VideoFrame> FrameGrabber::ExtractVideoFrameAsync(Windows::Foundation::TimeSpan position, bool exactSeek, int32_t maxFrameSkip, Windows::Storage::Streams::IBuffer targetBuffer)
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::IAsyncOperation<FFmpegInteropXWinUI::VideoFrame> FrameGrabber::ExtractNextVideoFrameAsync(Windows::Storage::Streams::IBuffer targetBuffer)
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::IAsyncOperation<FFmpegInteropXWinUI::VideoFrame> FrameGrabber::ExtractVideoFrameAsync(Windows::Foundation::TimeSpan position, bool exactSeek, int32_t maxFrameSkip)
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::IAsyncOperation<FFmpegInteropXWinUI::VideoFrame> FrameGrabber::ExtractVideoFrameAsync(Windows::Foundation::TimeSpan position, bool exactSeek)
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::IAsyncOperation<FFmpegInteropXWinUI::VideoFrame> FrameGrabber::ExtractVideoFrameAsync(Windows::Foundation::TimeSpan position)
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::IAsyncOperation<FFmpegInteropXWinUI::VideoFrame> FrameGrabber::ExtractNextVideoFrameAsync()
    {
        throw hresult_not_implemented();
    }
}
