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

		std::vector<const AVFilter*> AVFilters;
		std::vector<AVFilterContext*> AVFilterContexts;
		IVectorView<AvEffectDefinition^>^ currentEffectsDefintions;

		HRESULT AllocGraph()
		{
			if (graph)
				avfilter_graph_free(&this->graph);

			graph = avfilter_graph_alloc();

			if (graph)
				return S_OK;
			else return E_FAIL;
		}

		HRESULT AllocSource(AVPixelFormat swFrameFormat)
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

			AVPixelFormat sourceFormat = inputCodecCtx->pix_fmt;
			if (inputCodecCtx->pix_fmt != swFrameFormat)
			{
				sourceFormat = swFrameFormat;
			}

			hr = av_opt_set_int(avSource_ctx, "width", inputCodecCtx->width, AV_OPT_SEARCH_CHILDREN);
			hr = av_opt_set_int(avSource_ctx, "height", inputCodecCtx->height, AV_OPT_SEARCH_CHILDREN);
			hr = av_opt_set_int(avSource_ctx, "pix_fmt", sourceFormat, AV_OPT_SEARCH_CHILDREN);
			hr = av_opt_set_q(avSource_ctx, "time_base", inputCodecCtx->time_base, AV_OPT_SEARCH_CHILDREN);
			hr = av_opt_set_q(avSource_ctx, "frame_rate", inputCodecCtx->framerate, AV_OPT_SEARCH_CHILDREN);
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

		HRESULT AlocSourceAndSync(AVPixelFormat swFrameFormat)
		{
			//AVFilterContext *abuffer_ctx;
			auto hr = AllocSource(swFrameFormat);
			if (SUCCEEDED(hr))
			{
				hr = AllocSink();
			}

			return hr;
		}

		HRESULT InitFilterGraph(IVectorView<AvEffectDefinition^>^ effects, AVPixelFormat swFrameFormat)
		{
			//init graph
			int hr = 0;

			hr = AllocGraph();
			if (hr < 0)
				return E_FAIL;

			//alloc src and sink

			hr = AlocSourceAndSync(swFrameFormat);
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
		VideoFilter(AVCodecContext* m_inputCodecCtx)
		{
			this->inputCodecCtx = m_inputCodecCtx;
		}

		HRESULT AllocResources(IVectorView<AvEffectDefinition^>^ effects)
		{
			currentEffectsDefintions = effects;
			return S_OK;
		}

		HRESULT AddFrame(AVFrame* avFrame, AVFrame* outswFrame) override
		{
			AVFrame* targetFrame = NULL;
			if (avFrame->format == AV_PIX_FMT_D3D11)
			{
				av_hwframe_transfer_data(outswFrame, avFrame, 0);

				targetFrame = outswFrame;
			}
			else
			{
				if (outswFrame)
					av_frame_unref(outswFrame);
			}

			if (currentEffectsDefintions)
			{
				InitFilterGraph(currentEffectsDefintions, (AVPixelFormat)targetFrame->format);
				currentEffectsDefintions = nullptr;
			}

			auto hr = av_buffersrc_add_frame(avSource_ctx, targetFrame);

			return hr;
		}

		HRESULT GetFrame(AVFrame* avFrame) override
		{
			auto hr = av_buffersink_get_frame(avSink_ctx, avFrame);
			if (hr < 0)
			{
				return hr;
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



