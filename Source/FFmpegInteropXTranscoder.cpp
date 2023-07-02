#include "pch.h"
#include "FFmpegInteropXTranscoder.h"
#include "FFmpegInteropXTranscoder.g.cpp"

namespace winrt::FFmpegInteropX::implementation
{
    FFmpegInteropXTranscoder::FFmpegInteropXTranscoder(winrt::Windows::Storage::Streams::IRandomAccessStream const& inputStream, hstring const& transcodeFilter, winrt::Windows::Storage::Streams::IRandomAccessStream const& outputStream)
    {
        throw hresult_not_implemented();
    }
    winrt::Windows::Foundation::IAsyncActionWithProgress<winrt::FFmpegInteropX::FFmpegInteropXTranscoderProgress> FFmpegInteropXTranscoder::StartTranscodingAsync()
    {
        throw hresult_not_implemented();
    }
}
