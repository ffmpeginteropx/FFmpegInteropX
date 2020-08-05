#pragma once
#include "IAvEffect.h"
#include "AvEffectDefinition.h"
#include "AbstractEffectFactory.h"
#include <mutex>
#include <Windows.h>

extern "C"
{
#include <libavformat/avformat.h>
}

using namespace Windows::Foundation::Collections;

namespace FFmpegInterop
{
	ref class UncompressedHWVideoFrameProvider sealed
	{
		IAvEffect^ filter;
		AVFormatContext* m_pAvFormatCtx;
		AVCodecContext* m_pAvCodecCtx;
		AbstractEffectFactory^ m_effectFactory;
		bool hadFirstFrame;
		IVectorView<AvEffectDefinition^>^ pendingEffects;

	internal:

		UncompressedHWVideoFrameProvider(AVFormatContext* p_pAvFormatCtx, AVCodecContext* p_pAvCodecCtx, AbstractEffectFactory^ p_effectFactory)
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

		HRESULT GetHWFrameFromCodec(AVFrame* avhwFrame, AVFrame* avswFrame)
		{
			HRESULT hr = avcodec_receive_frame(m_pAvCodecCtx, avhwFrame);
			if (SUCCEEDED(hr))
			{				
				if (m_pAvCodecCtx->hw_device_ctx != NULL) {
					AVFrame* swFrame = av_frame_alloc();
					hr = av_hwframe_transfer_data(avswFrame, avhwFrame, 0);
				}
				
				hadFirstFrame = true;
				if (pendingEffects && pendingEffects->Size > 0)
				{
					filter = m_effectFactory->CreateEffect(pendingEffects);
					pendingEffects = nullptr;
				}
				if (filter)
				{
					hr = filter->AddFrame(avswFrame);
					if (SUCCEEDED(hr))
					{
						hr = filter->GetFrame(avswFrame);
					}
					if (FAILED(hr))
					{
						av_frame_unref(avswFrame);
					}
				}
			}
			return hr;
		}
	};
}