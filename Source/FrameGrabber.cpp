#include "pch.h"
#include "FrameGrabber.h"
#include "FrameGrabber.g.cpp"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropX::implementation
{
    using namespace Windows::Foundation;

    IAsyncOperation<FFmpegInteropX::FrameGrabber> FrameGrabber::CreateFromStreamAsync(Windows::Storage::Streams::IRandomAccessStream stream)
    {
        winrt::apartment_context caller; // Capture calling context.
        co_await winrt::resume_background();
        auto config = winrt::make_self<MediaSourceConfig>();
        config->IsFrameGrabber = true;
        config->VideoDecoderMode(VideoDecoderMode::ForceFFmpegSoftwareDecoder);

        auto result = FFmpegMediaSource::CreateFromStream(stream, config, nullptr);
        if (result == nullptr)
        {
            throw_hresult(E_FAIL);// ref new Exception(E_FAIL, "Could not create MediaStreamSource.");
        }
        if (result->VideoSampleProvider() == nullptr)
        {
            throw_hresult(E_INVALIDARG); //S "No video stream found in file (or no suitable decoder available).");
        }
        co_await caller;
        co_return winrt::make<FrameGrabber>(result);
    }

    IAsyncOperation<FFmpegInteropX::FrameGrabber> FrameGrabber::CreateFromUriAsync(hstring uri)
    {
        winrt::apartment_context caller; // Capture calling context.
        co_await winrt::resume_background();

        auto config = winrt::make_self<MediaSourceConfig>();
        config->IsFrameGrabber = true;
        config->VideoDecoderMode(VideoDecoderMode::ForceFFmpegSoftwareDecoder);

        auto result = FFmpegMediaSource::CreateFromUri(uri, config, nullptr);
        if (result == nullptr)
        {
            throw_hresult(E_FAIL);
        }
        if (result->CurrentVideoStream() == nullptr)
        {
            throw_hresult(E_INVALIDARG);
        }
        if (result->VideoSampleProvider() == nullptr)
        {
            throw_hresult(E_INVALIDARG);
        }
        co_await caller;
        co_return winrt::make<FrameGrabber>(result);
    }

    TimeSpan FrameGrabber::Duration()
    {
        return interopMSS->Duration();
    }

    int32_t FrameGrabber::DecodePixelWidth()
    {
        return decodePixelWidth;
    }

    void FrameGrabber::DecodePixelWidth(int32_t value)
    {
        decodePixelWidth = value;
    }

    int32_t FrameGrabber::DecodePixelHeight()
    {
        return decodePixelHeight;
    }

    void FrameGrabber::DecodePixelHeight(int32_t value)
    {
        decodePixelHeight = value;
    }

    FFmpegInteropX::VideoStreamInfo FrameGrabber::CurrentVideoStream()
    {
        return interopMSS->CurrentVideoStream();
    }

    IAsyncOperation<FFmpegInteropX::VideoFrame> FrameGrabber::ExtractVideoFrameAsync(TimeSpan position, bool exactSeek, int32_t maxFrameSkip, Windows::Storage::Streams::IBuffer targetBuffer)
    {
        auto strong = get_strong();
        PrepareDecoding(targetBuffer);
        winrt::apartment_context caller; // Capture calling context.
        co_await winrt::resume_background();

        auto cancellation = co_await get_cancellation_token();

        bool seekSucceeded = false;
        if (interopMSS->Duration().count() >= position.count())
        {
            TimeSpan actualPosition = position;
            seekSucceeded = SUCCEEDED(interopMSS->Seek(position, actualPosition, false));
        }

        int framesSkipped = 0;
        MediaStreamSample lastSample = nullptr;
        bool gotSample = true;

        while (true)
        {
            if (cancellation())
            {
                co_await caller;
                co_return nullptr;
            }

            auto sample = interopMSS->VideoSampleProvider()->GetNextSample(nullptr);
            if (sample == nullptr)
            {
                // if we hit end of stream, use last decoded sample (if any), otherwise fail
                if (lastSample != nullptr)
                {
                    sample = lastSample;
                    gotSample = false;
                }
                else
                {
                    throw_hresult(E_FAIL);
                }
            }
            else
            {
                lastSample = sample;
            }

            // if exact seek, continue decoding until we have the right sample
            if (gotSample && exactSeek && seekSucceeded && (position.count() - sample.Timestamp().count() > sample.Duration().count()) &&
                (maxFrameSkip <= 0 || framesSkipped < maxFrameSkip))
            {
                framesSkipped++;
                continue;
            }

            //  make sure we have a clean sample (key frame, no half interlaced frame)
            if (gotSample && !interopMSS->VideoSampleProvider()->IsCleanSample &&
                (maxFrameSkip <= 0 || framesSkipped < maxFrameSkip))
            {
                framesSkipped++;
                continue;
            }

            auto result = VideoFrame(
                sample.Buffer(),
                width,
                height,
                pixelAspectRatio,
                sample.Timestamp());

            co_await caller;
            co_return result;
        }
    }

    IAsyncOperation<FFmpegInteropX::VideoFrame> FrameGrabber::ExtractNextVideoFrameAsync(Windows::Storage::Streams::IBuffer targetBuffer)
    {
        auto strong = get_strong();
        PrepareDecoding(targetBuffer);
        winrt::apartment_context caller; // Capture calling context.
        co_await winrt::resume_background();

        VideoFrame result{ nullptr };
        auto sample = interopMSS->VideoSampleProvider()->GetNextSample(nullptr);
        if (sample)
        {
            result = VideoFrame(
                sample.Buffer(),
                width,
                height,
                pixelAspectRatio,
                sample.Timestamp());
        }

        co_await caller;
        co_return result;
    }

    IAsyncOperation<FFmpegInteropX::VideoFrame> FrameGrabber::ExtractVideoFrameAsync(TimeSpan position, bool exactSeek, int32_t maxFrameSkip)
    {
        return ExtractVideoFrameAsync(position, exactSeek, maxFrameSkip, nullptr);
    }

    IAsyncOperation<FFmpegInteropX::VideoFrame> FrameGrabber::ExtractVideoFrameAsync(TimeSpan position, bool exactSeek)
    {
        return ExtractVideoFrameAsync(position, exactSeek, 0, nullptr);
    }

    IAsyncOperation<FFmpegInteropX::VideoFrame> FrameGrabber::ExtractVideoFrameAsync(TimeSpan position)
    {
        return ExtractVideoFrameAsync(position, false, 0, nullptr);
    }

    IAsyncOperation<FFmpegInteropX::VideoFrame> FrameGrabber::ExtractNextVideoFrameAsync()
    {
        return ExtractNextVideoFrameAsync(nullptr);
    }

    void FrameGrabber::Close()
    {
        if (interopMSS)
            interopMSS->Close();
    }
}
