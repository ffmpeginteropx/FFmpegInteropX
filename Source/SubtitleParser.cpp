#include "pch.h"
#include "SubtitleParser.h"
#include "SubtitleParser.g.cpp"
#include "FFmpegMediaSource.h"
#include "MediaSourceConfig.h"

namespace winrt::FFmpegInteropX::implementation
{
    IAsyncOperation<winrt::FFmpegInteropX::SubtitleParser> SubtitleParser::ReadSubtitleAsync(winrt::Windows::Storage::Streams::IRandomAccessStream stream,
        hstring streamName,
        winrt::FFmpegInteropX::MediaSourceConfig config,
        winrt::Windows::Media::Core::VideoStreamDescriptor videoDescriptor)
    {
        winrt::apartment_context caller; // Capture calling context.
        co_await winrt::resume_background();

        auto parser = co_await winrt::FFmpegInteropX::implementation::FFmpegMediaSource::ReadExternalSubtitleStreamAsync(stream, streamName, config, videoDescriptor, nullptr, 0, false);
        auto result = parser.as<winrt::FFmpegInteropX::implementation::FFmpegMediaSource>();
        if (result == nullptr)
        {
            throw_hresult(E_FAIL);// ref new Exception(E_FAIL, "Could not parse stream");
        }
        if (result->SubtitleStreams().Size() != 1)
        {
            throw_hresult(E_INVALIDARG); //S "Nr of Subtitle stream found in stream in different than 1.");
        }
        co_await caller;
        co_return winrt::make<SubtitleParser>(result);
    }

    IAsyncOperation<winrt::FFmpegInteropX::SubtitleParser> SubtitleParser::ReadSubtitleAsync(winrt::Windows::Storage::Streams::IRandomAccessStream stream)
    {
        auto config = winrt::make<MediaSourceConfig>();
        return ReadSubtitleAsync(stream, L"", config, nullptr);
    }

    IAsyncOperation<winrt::FFmpegInteropX::SubtitleParser> SubtitleParser::ReadSubtitleAsync(Uri uri, hstring streamName, winrt::FFmpegInteropX::MediaSourceConfig config, winrt::Windows::Media::Core::VideoStreamDescriptor videoDescriptor)
    {
        auto stream = co_await (RandomAccessStreamReference::CreateFromUri(uri).OpenReadAsync());
        auto result = co_await ReadSubtitleAsync(stream, streamName, config, videoDescriptor);
        co_return result;
    }

    IAsyncOperation<winrt::FFmpegInteropX::SubtitleParser> SubtitleParser::ReadSubtitleAsync(Uri uri)
    {
        auto stream = co_await (RandomAccessStreamReference::CreateFromUri(uri).OpenReadAsync());
        auto result = co_await ReadSubtitleAsync(stream);
        co_return result;
    }

    winrt::FFmpegInteropX::SubtitleStreamInfo SubtitleParser::SubtitleTrack()
    {
        return interopMSS->SubtitleStreams().GetAt(0);
    }

    TimeSpan SubtitleParser::GetStreamDelay()
    {
        return interopMSS->GetStreamDelay(SubtitleTrack());
    }

    void SubtitleParser::SetStreamDelay(TimeSpan const& delay)
    {
        return interopMSS->SetStreamDelay(SubtitleTrack(), delay);
    }

    void SubtitleParser::Close()
    {
        if (interopMSS)
            interopMSS->Close();
    }
}
