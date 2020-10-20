#pragma once
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "IAvEffect.h"
#include "AvEffectDefinition.h"
#include <sstream>

extern "C"
{
#include "libavutil/channel_layout.h"
#include "libavutil/md5.h"
#include "libavutil/mem.h"
#include "libavutil/opt.h"
#include "libavutil/samplefmt.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
#include <libavfilter/avfilter.h>
#include <libswresample/swresample.h>
}

using namespace Windows::Foundation::Collections;
using namespace Windows::Media::Playback;
using namespace Windows::Foundation;
using namespace Platform;
using namespace Windows::Storage;

namespace FFmpegInterop {
	ref class VideoFilter : public IAvEffect
	{
		const AVFilter* AVSource;
		const AVFilter* AVSink;

		AVFilterGraph* graph;
		AVFilterContext* avSource_ctx, * avSink_ctx;

		AVCodecContext* inputCodecCtx;
		AVStream* inputStream;

		std::vector<const AVFilter*> AVFilters;
		std::vector<AVFilterContext*> AVFilterContexts;
		IVectorView<AvEffectDefinition^>^ currentEffectsDefintions;

		AVPixelFormat format;
		int width;
		int height;

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

			avSource_ctx = avfilter_graph_alloc_filter(graph, AVSource, "avSource_ctx");
			if (!avSource_ctx) {
				fprintf(stderr, "Could not allocate the abuffer instance.\n");
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

			avSink_ctx = avfilter_graph_alloc_filter(graph, AVSink, "sink");
			if (!avSink_ctx) {
				fprintf(stderr, "Could not allocate the abuffersink instance.\n");
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

		HRESULT InitFilterGraph(IVectorView<AvEffectDefinition^>^ effects, AVFrame* avFrame)
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


			//dynamic graph
			AVFilters.push_back(AVSource);
			AVFilterContexts.push_back(avSource_ctx);

			for (unsigned int i = 0; i < effects->Size; i++)
			{
				auto effectDefinition = effects->GetAt(i);

				auto effectName = StringUtils::PlatformStringToUtf8String(effectDefinition->FilterName);
				auto configString = StringUtils::PlatformStringToUtf8String(effectDefinition->Configuration);

				AVFilterContext* ctx;
				const AVFilter* filter;

				filter = avfilter_get_by_name(effectName.c_str());
				ctx = avfilter_graph_alloc_filter(graph, filter, configString.c_str());
				if (!filter)
				{
					return AVERROR_FILTER_NOT_FOUND;
				}
				if (avfilter_init_str(ctx, configString.c_str()) < 0)
				{
					return E_FAIL;
				}
				AVFilters.push_back(filter);
				AVFilterContexts.push_back(ctx);
			}


			AVFilters.push_back(AVSink);
			AVFilterContexts.push_back(avSink_ctx);


			hr = LinkGraph();
			return hr;
		}

		HRESULT LinkGraph()
		{
			int hr = 0;

			//link all except last item
			for (unsigned int i = 0; i < AVFilterContexts.size() - 1; i++)
			{
				if (hr >= 0)
					hr = avfilter_link(AVFilterContexts[i], 0, AVFilterContexts[i + 1], 0);
			}

			/* Configure the graph. */
			hr = avfilter_graph_config(graph, NULL);
			if (hr < 0) {
				return hr;
			}
			return S_OK;
		}


	internal:
		VideoFilter(AVCodecContext* m_inputCodecCtx, AVStream* inputStream)
		{
			this->inputCodecCtx = m_inputCodecCtx;
			this->inputStream = inputStream;
		}

		HRESULT AllocResources(IVectorView<AvEffectDefinition^>^ effects)
		{
			currentEffectsDefintions = effects;
			return S_OK;
		}

		HRESULT AddFrame(AVFrame* avFrame) override
		{	
			HRESULT hr = S_OK;

			if (currentEffectsDefintions)
			{
				hr = InitFilterGraph(currentEffectsDefintions, avFrame);
				currentEffectsDefintions = nullptr;
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

			if (currentEffectsDefintions)
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

			AVFilters.clear();
			AVFilterContexts.clear();
		}
	};
}



