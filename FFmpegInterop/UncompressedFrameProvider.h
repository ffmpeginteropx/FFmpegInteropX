#pragma once
#include "IAvEffect.h"
#include "AvEffectDefinition.h"
#include "AbstractEffectFactory.h"
#include <mutex>

extern "C"
{
#include <libavformat/avformat.h>
}

using namespace Windows::Foundation::Collections;

namespace FFmpegInterop
{
	ref class UncompressedFrameProvider sealed
	{
		IAvEffect^ filter;
		AVFormatContext* m_pAvFormatCtx;
		AVCodecContext* m_pAvCodecCtx;
		AbstractEffectFactory^ m_effectFactory;
		bool hadFirstFrame;
		IVectorView<AvEffectDefinition^>^ pendingEffects;

	internal:

		UncompressedFrameProvider(AVFormatContext* p_pAvFormatCtx, AVCodecContext* p_pAvCodecCtx, AbstractEffectFactory^ p_effectFactory)
		{
			m_pAvCodecCtx = p_pAvCodecCtx;
			m_pAvFormatCtx = p_pAvFormatCtx;
			m_effectFactory = p_effectFactory;
		}

		void UpdateFilter(IVectorView<AvEffectDefinition^>^ effects)
		{
			if (!hadFirstFrame)
			{
				pendingEffects = effects;
			}
			else
			{
				filter = m_effectFactory->CreateEffect(effects);
			}
		}

		void DisableFilter()
		{
			pendingEffects = nullptr;
			filter = nullptr;
		}

		HRESULT GetFrameFromCodec(AVFrame *avFrame)
		{
			HRESULT hr = avcodec_receive_frame(m_pAvCodecCtx, avFrame);
			if (SUCCEEDED(hr))
			{
				AVFrame* sourceFrame = avFrame;
				if (m_pAvCodecCtx->hw_device_ctx!=NULL) {
					AVFrame* swFrame = av_frame_alloc();
					
					auto err = av_hwframe_transfer_data(swFrame, avFrame, 0);
					sourceFrame = swFrame;
				}
				avFrame = sourceFrame;
				hadFirstFrame = true;
				if (pendingEffects && pendingEffects->Size > 0)
				{
					filter = m_effectFactory->CreateEffect(pendingEffects);
					pendingEffects = nullptr;
				}
				if (filter)
				{
					hr = filter->AddFrame(sourceFrame);
					if (SUCCEEDED(hr))
					{
						hr = filter->GetFrame(sourceFrame);
					}
					if (FAILED(hr))
					{
						av_frame_unref(sourceFrame);
					}
				}
			}
			return hr;
		}
	};

}