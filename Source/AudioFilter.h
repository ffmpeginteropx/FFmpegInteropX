#pragma once
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "IAvEffect.h"
#include "AvCodecContextHelpers.h"
#include <sstream>

extern "C"
{
#include "libavutil/channel_layout.h"
#include "libavutil/mem.h"
#include "libavutil/opt.h"
#include "libavutil/samplefmt.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
#include <libavfilter/avfilter.h>
#include <libswresample/swresample.h>
}

namespace FFmpegInteropX
{
	using namespace Windows::Foundation::Collections;
	using namespace Windows::Media::Playback;
	using namespace Windows::Foundation;
	using namespace Platform;
	using namespace Windows::Storage;

	class AudioFilter : public IAvEffect
	{
		const AVFilter* AVSource;
		const AVFilter* AVSink;

		AVFilterGraph* graph;
		AVFilterContext* avSource_ctx, * avSink_ctx;

		AVCodecContext* inputCodecCtx;

		String^ filterDefinition;
		bool isInitialized = false;
		char channel_layout_name[256];

		HRESULT InitFilterGraph(AVFrame* frame)
		{
			//init graph
			int hr = 0;

			hr = AllocGraph();
			if (hr < 0)
				return E_FAIL;

			//alloc src and sink
			hr = AlocSourceAndSync(frame);
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



		HRESULT AllocGraph()
		{
			if (graph)
				avfilter_graph_free(&this->graph);

			graph = avfilter_graph_alloc();

			if (graph)
				return S_OK;
			else return E_FAIL;
		}


		HRESULT AllocSource(AVFrame* frame)
		{
			int hr;

			auto layout = frame->channel_layout ? frame->channel_layout : inputCodecCtx->channel_layout;
			if (!layout)
				layout = AvCodecContextHelpers::GetDefaultChannelLayout(frame->channels);

			av_get_channel_layout_string(channel_layout_name, sizeof(channel_layout_name), frame->channels, layout);

			/* Create the abuffer filter;
			* it will be used for feeding the data into the graph. */
			AVSource = avfilter_get_by_name("abuffer");
			if (!AVSource)
			{
				fprintf(stderr, "Could not find the abuffer filter.\n");
				return AVERROR_FILTER_NOT_FOUND;
			}

			avSource_ctx = avfilter_graph_alloc_filter(graph, AVSource, "avSource_ctx");
			if (!avSource_ctx)
			{
				fprintf(stderr, "Could not allocate the abuffer instance.\n");
				return AVERROR(ENOMEM);
			}
			/* Set the filter options through the AVOptions API. */

			hr = av_opt_set_q(avSource_ctx, "time_base", inputCodecCtx->time_base, AV_OPT_SEARCH_CHILDREN);
			hr = av_opt_set_int(avSource_ctx, "sample_rate", frame->sample_rate, AV_OPT_SEARCH_CHILDREN);
			hr = av_opt_set(avSource_ctx, "sample_fmt", av_get_sample_fmt_name((AVSampleFormat)frame->format), AV_OPT_SEARCH_CHILDREN);
			hr = av_opt_set(avSource_ctx, "channel_layout", channel_layout_name, AV_OPT_SEARCH_CHILDREN);
			hr = av_opt_set_int(avSource_ctx, "channels", frame->channels, AV_OPT_SEARCH_CHILDREN);

			/* Now initialize the filter; we pass NULL options, since we have already
			* set all the options above. */
			hr = avfilter_init_str(avSource_ctx, NULL);
			return hr;
		}

		HRESULT AllocSink()
		{
			AVSink = avfilter_get_by_name("abuffersink");
			if (!AVSink)
			{
				fprintf(stderr, "Could not find the abuffersink filter.\n");
				return AVERROR_FILTER_NOT_FOUND;
			}

			avSink_ctx = avfilter_graph_alloc_filter(graph, AVSink, "abuffersink");
			if (!avSink_ctx)
			{
				fprintf(stderr, "Could not allocate the abuffersink instance.\n");
				return AVERROR(ENOMEM);
			}

			/* This filter takes no options. */
			return avfilter_init_str(avSink_ctx, NULL);

		}

		///There are 2 mandatory filters: the source, the sink.
		HRESULT AlocSourceAndSync(AVFrame* frame)
		{
			auto hr = AllocSource(frame);
			if (SUCCEEDED(hr))
			{
				hr = AllocSink();
			}
			return hr;
		}

	public:
		virtual ~AudioFilter()
		{
			avfilter_graph_free(&this->graph);
		}


		AudioFilter(AVCodecContext* m_inputCodecCtx, String^ filterDefinition)
		{
			this->inputCodecCtx = m_inputCodecCtx;
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
	};
}
