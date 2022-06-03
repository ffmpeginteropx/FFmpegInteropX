#pragma once
#include "FrameGrabber.g.h"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropXWinUI::implementation
{
    struct FrameGrabber : FrameGrabberT<FrameGrabber>
    {
        FrameGrabber() = default;

        static Windows::Foundation::IAsyncOperation<FFmpegInteropXWinUI::FrameGrabber> CreateFromStreamAsync(Windows::Storage::Streams::IRandomAccessStream stream);
        static Windows::Foundation::IAsyncOperation<FFmpegInteropXWinUI::FrameGrabber> CreateFromUriAsync(hstring uri);
        Windows::Foundation::TimeSpan Duration();
        int32_t DecodePixelWidth();
        void DecodePixelWidth(int32_t value);
        int32_t DecodePixelHeight();
        void DecodePixelHeight(int32_t value);
        FFmpegInteropXWinUI::VideoStreamInfo CurrentVideoStream();
        Windows::Foundation::IAsyncOperation<FFmpegInteropXWinUI::VideoFrame> ExtractVideoFrameAsync(Windows::Foundation::TimeSpan position, bool exactSeek, int32_t maxFrameSkip, Windows::Storage::Streams::IBuffer targetBuffer);
        Windows::Foundation::IAsyncOperation<FFmpegInteropXWinUI::VideoFrame> ExtractNextVideoFrameAsync(Windows::Storage::Streams::IBuffer targetBuffer);
        Windows::Foundation::IAsyncOperation<FFmpegInteropXWinUI::VideoFrame> ExtractVideoFrameAsync(Windows::Foundation::TimeSpan position, bool exactSeek, int32_t maxFrameSkip);
        Windows::Foundation::IAsyncOperation<FFmpegInteropXWinUI::VideoFrame> ExtractVideoFrameAsync(Windows::Foundation::TimeSpan position, bool exactSeek);
        Windows::Foundation::IAsyncOperation<FFmpegInteropXWinUI::VideoFrame> ExtractVideoFrameAsync(Windows::Foundation::TimeSpan position);
        Windows::Foundation::IAsyncOperation<FFmpegInteropXWinUI::VideoFrame> ExtractNextVideoFrameAsync();
    };
}
namespace winrt::FFmpegInteropXWinUI::factory_implementation
{
    struct FrameGrabber : FrameGrabberT<FrameGrabber, implementation::FrameGrabber>
    {
    };
}
