#pragma once
#include <pch.h>
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "IAvEffect.h"
#include <sstream>
#include "StringUtils.h"

extern "C"
{
#include <libavutil/md5.h>
#include <libavutil/mem.h>
#include <libavutil/opt.h>
#include <libavutil/samplefmt.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavfilter/avfilter.h>
#include <libavcodec/avcodec.h>
}

namespace FFmpegInteropX
{
	using namespace winrt::Windows::Foundation::Collections;
	using namespace winrt::Windows::Media::Playback;
	using namespace winrt::Windows::Foundation;

	using namespace winrt::Windows::Storage;

	class VideoFilter : public IAvEffect
	{
		const AVFilter* AVSource = NULL;
		const AVFilter* AVSink = NULL;

		AVFilterGraph* graph = NULL;
		AVFilterContext* avSource_ctx = NULL, * avSink_ctx = NULL;

		AVCodecContext* inputCodecCtx = NULL;
		AVStream* inputStream = NULL;

		winrt::hstring filterDefinition{};
		bool isInitialized = false;

		AVPixelFormat format;
		int width = 0;
		int height = 0;

		HRESULT AllocGraph()
		{
			if (graph)
				avfilter_graph_free(&this->graph);

			graph = avfilter_graph_alloc();

			if (graph)
				return S_OK;
			else return E_FAIL;
		}

		HRESULT AllocSource(AVFrame* avFrame)
		{
			AVDictionary* options_dict = NULL;

			int hr;

			/* Create the buffer filter;
			* it will be used for feeding the data into the graph. */
			AVSource = avfilter_get_by_name("buffer");
			if (!AVSource) {
				fprintf(stderr, "Could not find the buffer filter.\n");
				return AVERROR_FILTER_NOT_FOUND;
			}

			avSource_ctx = avfilter_graph_alloc_filter(graph, AVSource, "buffer");
			if (!avSource_ctx) {
				fprintf(stderr, "Could not allocate the buffer instance.\n");
				return AVERROR(ENOMEM);
			}

			format = (AVPixelFormat)avFrame->format;
			width = avFrame->width;
			height = avFrame->height;

			auto framerate = inputCodecCtx->framerate.num > 0 && inputCodecCtx->framerate.den > 0 ?
				inputCodecCtx->framerate : inputStream->avg_frame_rate;
			auto timeBase = inputCodecCtx->time_base.num > 0 && inputCodecCtx->time_base.den > 0 ?
				inputCodecCtx->time_base : inputStream->time_base;

			hr = av_opt_set_int(avSource_ctx, "width", width, AV_OPT_SEARCH_CHILDREN);
			hr = av_opt_set_int(avSource_ctx, "height", height, AV_OPT_SEARCH_CHILDREN);
			hr = av_opt_set_int(avSource_ctx, "pix_fmt", format, AV_OPT_SEARCH_CHILDREN);
			hr = av_opt_set_q(avSource_ctx, "time_base", timeBase, AV_OPT_SEARCH_CHILDREN);
			hr = av_opt_set_q(avSource_ctx, "frame_rate", framerate, AV_OPT_SEARCH_CHILDREN);
			hr = av_opt_set_q(avSource_ctx, "sar", inputCodecCtx->sample_aspect_ratio, AV_OPT_SEARCH_CHILDREN);

			/* Now initialize the filter; we pass NULL options, since we have already
			* set all the options above. */
			hr = avfilter_init_str(avSource_ctx, NULL);
			return hr;
		}

		HRESULT AllocSink()
		{
			AVSink = avfilter_get_by_name("buffersink");
			if (!AVSink) {
				fprintf(stderr, "Could not find the abuffersink filter.\n");
				return AVERROR_FILTER_NOT_FOUND;
			}

			avSink_ctx = avfilter_graph_alloc_filter(graph, AVSink, "buffersink");
			if (!avSink_ctx) {
				fprintf(stderr, "Could not allocate the buffersink instance.\n");
				return AVERROR(ENOMEM);
			}

			/* This filter takes no options. */
			return avfilter_init_str(avSink_ctx, NULL);

		}

		HRESULT AlocSourceAndSync(AVFrame* avFrame)
		{
			//AVFilterContext *abuffer_ctx;
			auto hr = AllocSource(avFrame);
			if (SUCCEEDED(hr))
			{
				hr = AllocSink();
			}

			return hr;
		}

		HRESULT InitFilterGraph(AVFrame* avFrame)
		{
			//init graph
			int hr = 0;

			hr = AllocGraph();
			if (hr < 0)
				return E_FAIL;

			//alloc src and sink

			hr = AlocSourceAndSync(avFrame);
			if (hr < 0)
				return E_FAIL;

			auto in = avfilter_inout_alloc();
			if (!in)
				return E_FAIL;

			in->name = av_strdup("in");
			in->filter_ctx = avSource_ctx;
			in->pad_idx = 0;
			in->next = NULL;

			auto out = avfilter_inout_alloc();
			if (!out)
				return E_FAIL;

			out->name = av_strdup("out");
			out->filter_ctx = avSink_ctx;
			out->pad_idx = 0;
			out->next = NULL;

			auto definition = StringUtils::PlatformStringToUtf8String(filterDefinition);
			hr = avfilter_graph_parse(graph, definition.c_str(), out, in, NULL);

			if (SUCCEEDED(hr))
			{
				hr = avfilter_graph_config(graph, NULL);
			}

			if (hr < 0) {
				return hr;
			}
			return S_OK;
		}


	public:
		VideoFilter(AVCodecContext* m_inputCodecCtx, AVStream* inputStream, winrt::hstring filterDefinition)
		{
			this->inputCodecCtx = m_inputCodecCtx;
			this->inputStream = inputStream;
			this->filterDefinition = filterDefinition;
		}

		HRESULT AddFrame(AVFrame* avFrame) override
		{
			HRESULT hr = S_OK;

			if (!isInitialized)
			{
				hr = InitFilterGraph(avFrame);
				isInitialized = true;
			}

			if (SUCCEEDED(hr))
			{
				if (avFrame && (avFrame->format != format || avFrame->width != width || avFrame->height != height))
				{
					// dynamic change of input size or format is not supported
					hr = E_FAIL;
				}
			}

			if (SUCCEEDED(hr))
			{
				hr = av_buffersrc_add_frame(avSource_ctx, avFrame);
			}

			return hr;
		}

		HRESULT GetFrame(AVFrame* avFrame) override
		{
			HRESULT hr;

			if (!isInitialized)
			{
				// not initialized: require frame
				hr = AVERROR(EAGAIN);
			}
			else
			{
				hr = av_buffersink_get_frame(avSink_ctx, avFrame);
			}

			return hr;
		}

	public:
		virtual ~VideoFilter()
		{
			avfilter_graph_free(&this->graph);
		}
	};
}



