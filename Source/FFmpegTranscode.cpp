#include "pch.h"
#include "FFmpegTranscode.h"
#include "FFmpegTranscode.g.cpp"
#include "FFmpegTranscodeInputCropRectangle.g.cpp"
#include "FFmpegTranscodeInputCropFrameEntry.g.cpp"
#include "FFmpegTranscodeInputTrimmingMarkerEntry.g.cpp"
#include "FFmpegTranscodeInput.g.cpp"
#include "FFmpegTranscodeOutput.g.cpp"

namespace winrt::FFmpegInteropX::implementation
{
    using namespace std;

    static AVCodecID GetCodecId(OutputType type)
    {
        switch (type)
        {
        case OutputType::Mp4:
            return AV_CODEC_ID_H264;
        case OutputType::Vp8:
            return AV_CODEC_ID_VP8;
        case OutputType::Vp9:
            return AV_CODEC_ID_VP9;
        default:
            throw_hresult(E_INVALIDARG);
        }
    }

    void FFmpegTranscode::throw_av_error(int ret)
    {
        char buffer[1024];
        av_strerror(ret, buffer, sizeof(buffer));
        av_log(nullptr, AV_LOG_ERROR, "%s\n", buffer);

        throw_hresult(E_FAIL);
    }
#define check_av_result(cmd) do { if((ret = cmd) < 0) throw_av_error(ret); } while(0)
#define check_av_pointer(ptr) do { if(!(ptr)) { av_log(nullptr, AV_LOG_ERROR, "Pointer returned as null.\n"); throw_hresult(E_FAIL); } } while(0)

    int FFmpegTranscode::FilterWriteFrame(AVFrame& filteredFrame, int64_t skippedPts,
        AVFormatContext& outputFormatContext, AVCodecContext& outputCodecContext, AVPacket& outputPacket, bool flush)
    {
        av_packet_unref(&outputPacket);

        if (filteredFrame.pts != AV_NOPTS_VALUE)
            filteredFrame.pts = av_rescale_q(av_rescale_q(filteredFrame.pts, filteredFrame.time_base, outputCodecContext.time_base) - skippedPts,
                outputFormatContext.streams[0]->time_base, filteredFrame.time_base);

        auto ret = avcodec_send_frame(&outputCodecContext, flush ? nullptr : &filteredFrame);
        while (ret >= 0)
        {
            ret = avcodec_receive_packet(&outputCodecContext, &outputPacket);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            {
                ret = 0;
                break;
            }

            outputPacket.stream_index = 0;
            outputPacket.dts = 0;
            ret = av_interleaved_write_frame(&outputFormatContext, &outputPacket);
        }

        return ret;
    }

    void FFmpegTranscode::SetEncodingParameters(AVCodecContext& ctx, FFmpegInteropX::FFmpegTranscodeOutput const& output)
    {
        int ret;

        SYSTEM_INFO si;
        GetNativeSystemInfo(&si);
        ctx.thread_count = min(16, si.dwNumberOfProcessors);
        ctx.slices = 8;

        switch (output.Type())
        {
        case OutputType::Mp4:
        {
            // crf, preset
            check_av_result(av_opt_set_int(ctx.priv_data, "crf", output.CRF(), 0));

            static const char* presets[] = { "ultrafast", "superfast", "veryfast", "faster", "fast", "medium", "slow", "slower", "veryslow" };
            check_av_result(av_opt_set(ctx.priv_data, "preset", presets[(int)output.Preset()], 0));
            break;
        }
        case OutputType::Vp8:
            throw_hresult(E_NOTIMPL);
            break;
        case OutputType::Vp9:
            // crf, bitrate = 0, speed = 1/2 (lower better), row-mt 1
            ctx.bit_rate = 0;
            ctx.qmin = output.CRF() - 2;
            ctx.qmax = output.CRF() + 2;
            ctx.qcompress = 1;

            check_av_result(av_opt_set_int(ctx.priv_data, "crf", output.CRF(), 0));
            check_av_result(av_opt_set_int(ctx.priv_data, "speed", 2, 0));
            check_av_result(av_opt_set_int(ctx.priv_data, "row-mt", 1, 0));
            check_av_result(av_opt_set_int(ctx.priv_data, "lag-in-frames", 25, 0));
            check_av_result(av_opt_set_int(ctx.priv_data, "cpu-used", 4, 0));
            check_av_result(av_opt_set_int(ctx.priv_data, "auto-alt-ref", 1, 0));
            check_av_result(av_opt_set_int(ctx.priv_data, "arnr-maxframes", 7, 0));
            check_av_result(av_opt_set_int(ctx.priv_data, "arnr-strength", 4, 0));
            check_av_result(av_opt_set_int(ctx.priv_data, "aq-mode", 4, 0));
            check_av_result(av_opt_set_int(ctx.priv_data, "tile-columns", 6, 0));
            check_av_result(av_opt_set_int(ctx.priv_data, "tile-rows", 2, 0));
            break;
        }
    }

    template<typename T>
    static FFmpegInteropX::FFmpegTranscodeInputCropRectangle GetCropRectangle(T cropFrameIt, T nextCropFrameIt, T endIt, int64_t frameNumber)
    {
        auto cropFrame = *cropFrameIt;
        assert(frameNumber >= cropFrame.FrameNumber());

        if (nextCropFrameIt == endIt)
            return cropFrame.CropRectangle();

        auto nextCropFrame = *nextCropFrameIt;
        assert(frameNumber < nextCropFrame.FrameNumber());

        auto f = (double)(frameNumber - cropFrame.FrameNumber()) / (nextCropFrame.FrameNumber() - cropFrame.FrameNumber());
        auto center_x = cropFrame.CropRectangle().CenterX() + f * (nextCropFrame.CropRectangle().CenterX() - cropFrame.CropRectangle().CenterX());
        auto center_y = cropFrame.CropRectangle().CenterY() + f * (nextCropFrame.CropRectangle().CenterY() - cropFrame.CropRectangle().CenterY());
        auto width = cropFrame.CropRectangle().Width() + f * (nextCropFrame.CropRectangle().Width() - cropFrame.CropRectangle().Width());
        auto height = cropFrame.CropRectangle().Height() + f * (nextCropFrame.CropRectangle().Height() - cropFrame.CropRectangle().Height());

        return { (int)center_x, (int)center_y, (int)width, (int)height };
    }

    void FFmpegTranscode::Run(FFmpegInteropX::FFmpegTranscodeInput const& input, FFmpegInteropX::FFmpegTranscodeOutput const& output)
    {
        int ret;

        // open input
        AutoReleasePtr<AVFormatContext, avformat_close_input> inputFormatContext;
        check_av_result(avformat_open_input(&inputFormatContext, StringUtils::PlatformStringToUtf8String(input.FileName()).c_str(),
            nullptr, nullptr));

        check_av_result(avformat_find_stream_info(&*inputFormatContext, nullptr));

        auto inputVideoStream = inputFormatContext->streams[input.VideoStreamIndex()];

        auto inputCodecPar = inputVideoStream->codecpar;
        auto inputCodec = avcodec_find_decoder(inputCodecPar->codec_id);
        check_av_pointer(inputCodec);

        AutoReleasePtr<AVCodecContext, avcodec_free_context> inputCodecContext = avcodec_alloc_context3(inputCodec);
        check_av_result(avcodec_parameters_to_context(&*inputCodecContext, inputCodecPar));

        inputCodecContext->framerate = av_guess_frame_rate(&*inputFormatContext, inputVideoStream, nullptr);
        inputCodecContext->pkt_timebase = inputVideoStream->time_base;

        check_av_result(avcodec_open2(&*inputCodecContext, inputCodec, nullptr));
        
        av_dump_format(&*inputFormatContext, 0, StringUtils::PlatformStringToUtf8String(input.FileName()).c_str(), 0);

        // open output
        AutoReleasePtr<AVFormatContext, avformat_free_context> outputFormatContext;
        check_av_result(avformat_alloc_output_context2(&outputFormatContext, nullptr, nullptr,
            StringUtils::PlatformStringToUtf8String(output.FileName()).c_str()));

        outputFormatContext->avoid_negative_ts = AVFMT_AVOID_NEG_TS_MAKE_NON_NEGATIVE;

        // build output codec
        auto outputCodec = avcodec_find_encoder(GetCodecId(output.Type()));
        check_av_pointer(outputCodec);

        AutoReleasePtr<AVCodecContext, avcodec_free_context> outputCodecContext = avcodec_alloc_context3(outputCodec);
        check_av_pointer(outputCodecContext);

        SetEncodingParameters(*outputCodecContext, output);
        outputCodecContext->width = (int)output.PixelSize().Width;
        outputCodecContext->height = (int)output.PixelSize().Height;
        outputCodecContext->framerate = inputCodecContext->framerate;
        outputCodecContext->time_base = av_inv_q(inputCodecContext->framerate);
        outputCodecContext->gop_size = 240;  // I-frame interval
        outputCodecContext->max_b_frames = 1;
        outputCodecContext->pix_fmt = inputCodecContext->pix_fmt;

        if (outputFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
            outputCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

        check_av_result(avcodec_open2(&*outputCodecContext, outputCodec, nullptr));

        auto outputVideoStream = avformat_new_stream(&*outputFormatContext, nullptr);
        check_av_pointer(outputVideoStream);

        check_av_result(avcodec_parameters_from_context(outputVideoStream->codecpar, &*outputCodecContext));
        outputVideoStream->time_base = outputCodecContext->time_base;

        av_dump_format(&*outputFormatContext, 0, StringUtils::PlatformStringToUtf8String(output.FileName()).c_str(), 1);

        // open the output file
        if (!(outputFormatContext->oformat->flags & AVFMT_NOFILE))
            check_av_result(avio_open(&outputFormatContext->pb, StringUtils::PlatformStringToUtf8String(output.FileName()).c_str(), AVIO_FLAG_WRITE));

        // write the stream header
        check_av_result(avformat_write_header(&*outputFormatContext, nullptr));

        // build filter graph
        AutoReleasePtr<AVFilterInOut, avfilter_inout_free> filterInputs = avfilter_inout_alloc();
        check_av_pointer(filterInputs);
        AutoReleasePtr<AVFilterInOut, avfilter_inout_free> filterOutputs = avfilter_inout_alloc();
        check_av_pointer(filterOutputs);
        AutoReleasePtr<AVFilterGraph, avfilter_graph_free> filterGraph = avfilter_graph_alloc();
        check_av_pointer(filterGraph);

        auto bufferSource = avfilter_get_by_name("buffer");
        check_av_pointer(bufferSource);
        auto bufferSink = avfilter_get_by_name("buffersink");
        check_av_pointer(bufferSink);

        auto args = std::format("video_size={}x{}:pix_fmt={}:time_base={}/{}:pixel_aspect={}/{}",
            inputCodecContext->width, inputCodecContext->height, (int)inputCodecContext->pix_fmt,
            inputCodecContext->pkt_timebase.num, inputCodecContext->pkt_timebase.den,
            inputCodecContext->sample_aspect_ratio.num, inputCodecContext->sample_aspect_ratio.den);

        AVFilterContext* buffersrc_ctx = nullptr, * buffersink_ctx = nullptr;
        check_av_result(avfilter_graph_create_filter(&buffersrc_ctx, bufferSource, "in",
            args.c_str(), nullptr, &*filterGraph));
        check_av_result(avfilter_graph_create_filter(&buffersink_ctx, bufferSink, "out",
            nullptr, nullptr, &*filterGraph));
        check_av_result(av_opt_set_bin(buffersink_ctx, "pix_fmts",
            (uint8_t*)&outputCodecContext->pix_fmt, sizeof(outputCodecContext->pix_fmt), AV_OPT_SEARCH_CHILDREN));

        // endpoints for the filter graph
        filterInputs->name = av_strdup("out");
        filterInputs->filter_ctx = buffersink_ctx;
        filterInputs->pad_idx = 0;
        filterInputs->next = nullptr;

        filterOutputs->name = av_strdup("in");
        filterOutputs->filter_ctx = buffersrc_ctx;
        filterOutputs->pad_idx = 0;
        filterOutputs->next = nullptr;

        auto filterSpec = std::format("[in]crop[cropped];[cropped]scale={}:{}[scaled];[scaled]setsar=1:1[out]",
            outputCodecContext->width, outputCodecContext->height);

        check_av_result(avfilter_graph_parse_ptr(&*filterGraph, filterSpec.c_str(), &filterInputs, &filterOutputs, nullptr));
        check_av_result(avfilter_graph_config(&*filterGraph, nullptr));

        auto cropFilterContext = avfilter_graph_get_filter(&*filterGraph, "Parsed_crop_0");
        check_av_pointer(cropFilterContext);

        AutoReleasePtr<AVPacket, av_packet_unref> inputPacket = av_packet_alloc();
        AutoReleasePtr<AVFrame, av_frame_free> inputFrame = av_frame_alloc();
        AutoReleasePtr<AVPacket, av_packet_unref> outputPacket = av_packet_alloc();
        AutoReleasePtr<AVFrame, av_frame_free> filteredFrame = av_frame_alloc();

        auto cropFrames = input.CropFrames();
        auto cropFrameIt = cropFrames.begin();
        auto nextCropFrameIt = cropFrameIt + 1;

        auto trimmingMarkers = input.TrimmingMarkers();
        auto trimmingMarkerIt = trimmingMarkers.begin();
        auto nextTrimmingMarkerIt = trimmingMarkerIt + 1;

        int64_t inputFrameNumber = 0, outputFrameNumber = 0;
        int64_t skippedPts = 0;

        jthread filterEncoderThread{ [&](stop_token st) {
            // pull filtered frames from the filter graph
            while (1)
            {
                ret = av_buffersink_get_frame(buffersink_ctx, &*filteredFrame);
                if (ret < 0)
                {
                    // if no more frames, rewrite the code to 0 to show it as normal completion
                    if (ret == AVERROR(EAGAIN))
                        if (st.stop_requested())
                            return;     // we'll never get more frames
                        else
                            continue;
                    if (ret == AVERROR_EOF)
                        return;
                    break;
                }

                // write the filtered frame to the output file
                filteredFrame->time_base = av_buffersink_get_time_base(buffersink_ctx);
                filteredFrame->pict_type = AV_PICTURE_TYPE_NONE;

                ret = FilterWriteFrame(*filteredFrame, skippedPts, *outputFormatContext, *outputCodecContext, *outputPacket, false);
                frameOutputProgress(*this, outputFrameNumber);

                av_frame_unref(&*filteredFrame);
                if (ret < 0)
                    break;
            }
            if (ret < 0)
                check_av_result(ret);
        } };

        // transcode
        while (true)
        {
            // end?
            if (nextTrimmingMarkerIt == trimmingMarkers.end() && (*trimmingMarkerIt).TrimAfter())
                break;

            if (av_read_frame(&*inputFormatContext, &*inputPacket) < 0)
                break;

            if (inputPacket->stream_index == input.VideoStreamIndex())
            {
                check_av_result(avcodec_send_packet(&*inputCodecContext, &*inputPacket));

                while (ret >= 0)
                {
                    ret = avcodec_receive_frame(&*inputCodecContext, &*inputFrame);
                    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                        break;
                    else if (ret < 0)
                        check_av_result(ret);

                    inputFrame->pts = inputFrame->best_effort_timestamp;

                    // handle trimming
                    if (nextTrimmingMarkerIt != trimmingMarkers.end() && inputFrameNumber >= (*nextTrimmingMarkerIt).FrameNumber())
                    {
                        trimmingMarkerIt = nextTrimmingMarkerIt;
                        ++nextTrimmingMarkerIt;
                    }
                    ++inputFrameNumber;
                    if ((*trimmingMarkerIt).TrimAfter())
                    {
                        skippedPts += 1;
                        continue;
                    }

                    // update the crop rectangle
                    while (nextCropFrameIt != cropFrames.end() && outputFrameNumber >= (*nextCropFrameIt).FrameNumber())
                    {
                        cropFrameIt = nextCropFrameIt;
                        ++nextCropFrameIt;
                    }
                    auto cropRectangle = GetCropRectangle(cropFrameIt, nextCropFrameIt, cropFrames.end(), outputFrameNumber);
                    ++outputFrameNumber;

                    // crop the frame
                    check_av_result(avfilter_graph_send_command(&*filterGraph, "Parsed_crop_0", "x", std::to_string(cropRectangle.CenterX() - cropRectangle.Width() / 2).c_str(), nullptr, 0, 0));
                    check_av_result(avfilter_graph_send_command(&*filterGraph, "Parsed_crop_0", "y", std::to_string(cropRectangle.CenterY() - cropRectangle.Height() / 2).c_str(), nullptr, 0, 0));
                    check_av_result(avfilter_graph_send_command(&*filterGraph, "Parsed_crop_0", "w", std::to_string(cropRectangle.Width()).c_str(), nullptr, 0, 0));
                    check_av_result(avfilter_graph_send_command(&*filterGraph, "Parsed_crop_0", "h", std::to_string(cropRectangle.Height()).c_str(), nullptr, 0, 0));

                    // push the decoded frame into the filter graph
                    check_av_result(av_buffersrc_add_frame_flags(buffersrc_ctx, &*inputFrame, 0));

                    // pull filtered frames from the filter graph
                    while (1)
                    {
                        ret = av_buffersink_get_frame(buffersink_ctx, &*filteredFrame);
                        if (ret < 0)
                        {
                            // if no more frames, rewrite the code to 0 to show it as normal completion
                            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                                ret = 0;
                            break;
                        }

                        // write the filtered frame to the output file
                        filteredFrame->time_base = av_buffersink_get_time_base(buffersink_ctx);
                        filteredFrame->pict_type = AV_PICTURE_TYPE_NONE;

                        ret = FilterWriteFrame(*filteredFrame, skippedPts, *outputFormatContext, *outputCodecContext, *outputPacket, false);
                        frameOutputProgress(*this, outputFrameNumber);

                        av_frame_unref(&*filteredFrame);
                        if (ret < 0)
                            break;
                    }

                    if (ret < 0)
                        check_av_result(ret);
                }
            }

            av_packet_unref(&*inputPacket);
        }

        // flush stuff
        if (av_buffersrc_add_frame_flags(buffersrc_ctx, nullptr, 0) >= 0)
        {
            // nothing to do on this thread
        }

        filterEncoderThread.request_stop();
        filterEncoderThread.join();

        check_av_result(av_write_trailer(&*outputFormatContext));
        check_av_result(avio_closep(&outputFormatContext->pb));
    }

    FFmpegTranscode::~FFmpegTranscode()
    {
        Close();
    }

    void FFmpegTranscode::Close()
    {
    }
}
