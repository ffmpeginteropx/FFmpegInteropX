#include "pch.h"
#include "FrameGrabber.h"
#include "FrameGrabber.g.cpp"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropX::implementation
{
    static int FileStreamRead(void* ptr, uint8_t* buf, int bufSize);
    static int FileStreamWrite(void* ptr, uint8_t* buf, int bufSize);
    static int64_t FileStreamSeek(void* ptr, int64_t pos, int whence);

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

    IAsyncOperation<bool> FrameGrabber::CombineAsync(Windows::Storage::Streams::IRandomAccessStream outStream, Windows::Storage::Streams::IRandomAccessStream audioStream, Windows::Storage::Streams::IRandomAccessStream videoStream, winrt::hstring format)
    {
        co_return co_await concurrency::create_task(concurrency::task<bool>([&outStream, &audioStream, &videoStream, &format] {
            int hr = 0;

            winrt::com_ptr<IStream> audioFileStreamData = { nullptr };
            winrt::com_ptr<IStream> videoFileStreamData = { nullptr };
            winrt::com_ptr<IStream> outputFileStreamData = { nullptr };

            unsigned char* audioFileStreamBuffer = nullptr;
            unsigned char* videoFileStreamBuffer = nullptr;
            unsigned char* outputFileStreamBuffer = nullptr;

            hr = CreateStreamOverRandomAccessStream(reinterpret_cast<::IUnknown*>(winrt::get_abi(audioStream)), IID_PPV_ARGS(&audioFileStreamData));
            hr = CreateStreamOverRandomAccessStream(reinterpret_cast<::IUnknown*>(winrt::get_abi(videoStream)), IID_PPV_ARGS(&videoFileStreamData));
            hr = CreateStreamOverRandomAccessStream(reinterpret_cast<::IUnknown*>(winrt::get_abi(outStream)), IID_PPV_ARGS(&outputFileStreamData));

            audioFileStreamBuffer = (unsigned char*)av_malloc(16384);
            videoFileStreamBuffer = (unsigned char*)av_malloc(16384);
            outputFileStreamBuffer = (unsigned char*)av_malloc(16384);

            AVFormatContext* ifmt_audio_ctx = avformat_alloc_context(), * ifmt_video_ctx = avformat_alloc_context(), * ofmt_ctx = nullptr;
            AVIOContext* avIOCtxAudio = avio_alloc_context(audioFileStreamBuffer, 16384, 0, (void*)winrt::get_abi(audioFileStreamData), FileStreamRead, 0, FileStreamSeek);;
            AVIOContext* avIOCtxVideo = avio_alloc_context(videoFileStreamBuffer, 16384, 0, (void*)winrt::get_abi(videoFileStreamData), FileStreamRead, 0, FileStreamSeek);;
            AVIOContext* avIOCtxOut = avio_alloc_context(outputFileStreamBuffer, 16384, AVIO_FLAG_READ_WRITE, (void*)winrt::get_abi(outputFileStreamData), FileStreamRead, FileStreamWrite, FileStreamSeek);;
            ifmt_audio_ctx->pb = avIOCtxAudio;
            ifmt_audio_ctx->flags |= AVFMT_FLAG_CUSTOM_IO;

            ifmt_video_ctx->pb = avIOCtxVideo;
            ifmt_video_ctx->flags |= AVFMT_FLAG_CUSTOM_IO;


            int ret;
            if ((ret = avformat_open_input(&ifmt_video_ctx, NULL, NULL, NULL)))
            {
                char buf[256];
                av_strerror(ret, buf, sizeof(buf));
                printf("error :%s,ret:%d\n", buf, ret);
                return false;
            }

            if ((ret = avformat_open_input(&ifmt_audio_ctx, NULL, NULL, NULL)))
            {
                char buf[256];
                av_strerror(ret, buf, sizeof(buf));
                printf("error :%s,ret:%d\n", buf, ret);
                return false;
            }


            avformat_find_stream_info(ifmt_audio_ctx, nullptr);
            avformat_find_stream_info(ifmt_video_ctx, nullptr);

            const char* charStr = nullptr;
            std::wstring uriW(format.begin());
            std::string uriA(uriW.begin(), uriW.end());

            charStr = uriA.c_str();
            avformat_alloc_output_context2(&ofmt_ctx, NULL, charStr, NULL);
            if (!ofmt_ctx) {
                fprintf(stderr, "Could not create output context\n");
                ret = AVERROR_UNKNOWN;
                return false;
            }
            ofmt_ctx->pb = avIOCtxOut;
            ofmt_ctx->flags |= AVFMT_FLAG_CUSTOM_IO;


            AVStream* in_audio_stream = ifmt_audio_ctx->streams[0];
            AVStream* in_video_stream = ifmt_video_ctx->streams[0];
            AVCodecParameters* in_audio_codecpar = in_audio_stream->codecpar;
            AVCodecParameters* in_video_codecpar = in_video_stream->codecpar;

            AVStream* out_audio_stream, * out_video_stream;

            const AVCodec* decVideo = avcodec_find_decoder(in_video_stream->codecpar->codec_id);
            out_video_stream = avformat_new_stream(ofmt_ctx, decVideo);
            if (!out_video_stream) {
                fprintf(stderr, "Failed allocating output stream\n");
                ret = AVERROR_UNKNOWN;
                return false;
            }

            const AVCodec* decAudio = avcodec_find_decoder(in_audio_stream->codecpar->codec_id);
            out_audio_stream = avformat_new_stream(ofmt_ctx, decAudio);
            if (!out_audio_stream) {
                fprintf(stderr, "Failed allocating output stream\n");
                ret = AVERROR_UNKNOWN;
                return false;
            }
            out_audio_stream->avg_frame_rate.den = in_audio_stream->avg_frame_rate.den;
            out_audio_stream->avg_frame_rate.num = in_audio_stream->avg_frame_rate.num;

            out_audio_stream->pts_wrap_bits = in_audio_stream->pts_wrap_bits;
            out_video_stream->pts_wrap_bits = out_video_stream->pts_wrap_bits;


            out_video_stream->avg_frame_rate.den = in_video_stream->avg_frame_rate.den;
            out_video_stream->avg_frame_rate.num = in_video_stream->avg_frame_rate.num;

            out_audio_stream->start_time = in_audio_stream->start_time;
            out_audio_stream->duration = in_audio_stream->duration;
            out_video_stream->start_time = in_video_stream->start_time;
            out_video_stream->duration = in_video_stream->duration;

            out_audio_stream->time_base.den = in_audio_stream->time_base.den;
            out_audio_stream->time_base.num = in_audio_stream->time_base.num;

            out_video_stream->time_base.den = in_video_stream->time_base.den;
            out_video_stream->time_base.num = in_video_stream->time_base.num;

            unsigned int tagVideo = 0;
            if (av_codec_get_tag2(ofmt_ctx->oformat->codec_tag, decVideo->id, &tagVideo) == 0) {
                av_log(NULL, AV_LOG_ERROR, "could not find codec tag for codec id %d, default to 0.\n", decVideo->id);
            }
            unsigned int tagAudio = 0;
            if (av_codec_get_tag2(ofmt_ctx->oformat->codec_tag, decAudio->id, &tagAudio) == 0) {
                av_log(NULL, AV_LOG_ERROR, "could not find codec tag for codec id %d, default to 0.\n", decAudio->id);
            }

            ret = avcodec_parameters_copy(out_video_stream->codecpar, in_video_codecpar);
            if (ret < 0) {
                fprintf(stderr, "Failed to copy codec parameters\n");
                return false;
            }
            ret = avcodec_parameters_copy(out_audio_stream->codecpar, in_audio_codecpar);
            if (ret < 0) {
                fprintf(stderr, "Failed to copy codec parameters\n");
                return false;
            }
            out_video_stream->codecpar->codec_tag = tagVideo;
            out_audio_stream->codecpar->codec_tag = tagAudio;

            ret = avformat_write_header(ofmt_ctx, NULL);
            if (ret < 0) {
                char buf[256];
                av_strerror(ret, buf, sizeof(buf));
                printf("error :%s,ret:%d\n", buf, ret);

                fprintf(stderr, "Error occurred when opening output file\n");
                return false;
            }

            AVPacket pkt;
            while (true)
            {
                ret = av_read_frame(ifmt_audio_ctx, &pkt);
                if (ret < 0)
                    break;

                if (pkt.stream_index != 0) {
                    av_packet_unref(&pkt);
                    continue;
                }

                pkt.stream_index = 1;
                ret = av_interleaved_write_frame(ofmt_ctx, &pkt);
                if (ret < 0) {
                    fprintf(stderr, "Error muxing packet\n");
                    return false;
                    break;
                }
                av_packet_unref(&pkt);
            }
            while (true)
            {
                ret = av_read_frame(ifmt_video_ctx, &pkt);
                if (ret < 0)
                    break;

                if (pkt.stream_index != 0) {
                    av_packet_unref(&pkt);
                    continue;
                }

                pkt.stream_index = 0;
                ret = av_interleaved_write_frame(ofmt_ctx, &pkt);
                if (ret < 0) {
                    fprintf(stderr, "Error muxing packet\n");
                    return false;
                    break;
                }
                av_packet_unref(&pkt);
            }

            av_write_trailer(ofmt_ctx);

            avformat_close_input(&ifmt_audio_ctx);
            avformat_close_input(&ifmt_video_ctx);

            return true;
            }));
    }

    IAsyncOperation<FFmpegInteropX::FrameGrabber> FrameGrabber::CreateFromUriAsync(hstring uri)
    {
        winrt::apartment_context caller; // Capture calling context.
        co_await winrt::resume_background();

        auto config = winrt::make_self<MediaSourceConfig>();
        config->IsFrameGrabber = true;
        config->VideoDecoderMode(VideoDecoderMode::ForceFFmpegSoftwareDecoder);

        auto result = FFmpegMediaSource::CreateFromUri(uri, config);
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

            auto sample = interopMSS->VideoSampleProvider()->GetNextSample();
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
        auto sample = interopMSS->VideoSampleProvider()->GetNextSample();
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

    // Static function to read file stream and pass data to FFmpeg. Credit to Philipp Sch http://www.codeproject.com/Tips/489450/Creating-Custom-FFmpeg-IO-Context
    static int FileStreamRead(void* ptr, uint8_t* buf, int bufSize)
    {
        IStream* pStream = reinterpret_cast<IStream*>(ptr);
        ULONG bytesRead = 0;
        HRESULT hr = pStream->Read(buf, bufSize, &bytesRead);

        if (FAILED(hr))
        {
            return -1;
        }

        // If we succeed but don't have any bytes, assume end of file
        if (bytesRead == 0)
        {
            return AVERROR_EOF;  // Let FFmpeg know that we have reached eof
        }

        return bytesRead;
    }

    // Static function to seek in file stream. Credit to Philipp Sch http://www.codeproject.com/Tips/489450/Creating-Custom-FFmpeg-IO-Context
    static int64_t FileStreamSeek(void* ptr, int64_t pos, int whence)
    {
        IStream* pStream = reinterpret_cast<IStream*>(ptr);
        if (whence == AVSEEK_SIZE)
        {
            // get stream size
            STATSTG status;
            if (FAILED(pStream->Stat(&status, STATFLAG_NONAME)))
            {
                return -1;
            }
            return status.cbSize.QuadPart;
        }
        else
        {
            LARGE_INTEGER in;
            in.QuadPart = pos;
            ULARGE_INTEGER out = { 0 };

            if (FAILED(pStream->Seek(in, whence, &out)))
            {
                return -1;
            }

            return out.QuadPart; // Return the new position:
        }
    }

    static int FileStreamWrite(void* ptr, uint8_t* buf, int bufSize)
    {
        IStream* pStream = reinterpret_cast<IStream*>(ptr);
        ULONG bytesRead = 0;
        HRESULT hr = pStream->Write(buf, bufSize, &bytesRead);

        if (FAILED(hr))
        {
            return -1;
        }

        // If we succeed but don't have any bytes, assume end of file
        if (bytesRead == 0)
        {
            return AVERROR_EOF;  // Let FFmpeg know that we have reached eof
        }

        return bytesRead;
    }
}
