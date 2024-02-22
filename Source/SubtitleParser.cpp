#include "pch.h"
#include "SubtitleParser.h"
#include "SubtitleParser.g.cpp"

namespace winrt::FFmpegInteropX::implementation
{
    winrt::Windows::Foundation::IAsyncOperation<winrt::FFmpegInteropX::SubtitleParser> SubtitleParser::AddExternalSubtitleAsync(winrt::Windows::Storage::Streams::IRandomAccessStream stream, hstring streamName)
    {
        winrt::apartment_context caller; // Capture calling context.
        co_await winrt::resume_background();
        auto config = winrt::make_self<MediaSourceConfig>();
        config->IsFrameGrabber = true;
        config->Video().VideoDecoderMode(VideoDecoderMode::ForceFFmpegSoftwareDecoder);

        auto result = FFmpegMediaSource::CreateFromStream(stream, config, nullptr, 0, false);
        if (result == nullptr)
        {
            throw_hresult(E_FAIL);// ref new Exception(E_FAIL, "Could not create MediaStreamSource.");
        }
        if (result->SubtitleStreams().Size() != 1)
        {
            throw_hresult(E_INVALIDARG); //S "No subtitle stream found in stream.");
        }
        co_await caller;
        co_return winrt::make<FrameGrabber>(result);

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
