#include "pch.h"
#include "SubtitleParser.h"
#include "SubtitleParser.g.cpp"
#include "FFmpegMediaSource.h">

namespace winrt::FFmpegInteropX::implementation
{
    winrt::Windows::Foundation::IAsyncOperation<winrt::FFmpegInteropX::SubtitleParser> SubtitleParser::AddExternalSubtitleAsync(winrt::Windows::Storage::Streams::IRandomAccessStream stream,
        hstring streamName,
        winrt::FFmpegInteropX::MediaSourceConfig config,
        winrt::Windows::Media::Core::VideoStreamDescriptor videoDescriptor,
        uint64_t windowId,
        bool useHdr)
    {
        winrt::apartment_context caller; // Capture calling context.
        co_await winrt::resume_background();

        auto result = (winrt::FFmpegInteropX::implementation::FFmpegMediaSource::AddExternalSubtitleAsyncInternal(stream, streamName, config, nullptr, nullptr, 0, false)).as<winrt::FFmpegInteropX::implementation::FFmpegMediaSource>();
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

    winrt::Windows::Foundation::IAsyncOperation<winrt::FFmpegInteropX::SubtitleParser> SubtitleParser::AddExternalSubtitleAsync(winrt::Windows::Storage::Streams::IRandomAccessStream stream)
    {
        throw hresult_not_implemented();
    }

    winrt::FFmpegInteropX::SubtitleStreamInfo SubtitleParser::SubtitleTrack()
    {
        return interopMSS->SubtitleStreams().GetAt(0);
    }

    winrt::Windows::Foundation::TimeSpan SubtitleParser::GetStreamDelay()
    {
        return interopMSS->GetStreamDelay(SubtitleTrack());
    }

    void SubtitleParser::SetStreamDelay(winrt::Windows::Foundation::TimeSpan const& delay)
    {
        return interopMSS->SetStreamDelay(SubtitleTrack(), delay);
    }

    void SubtitleParser::Close()
    {
        if (interopMSS)
            interopMSS->Close();
    }
}
