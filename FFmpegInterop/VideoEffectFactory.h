#pragma once
#include "AbstractEffectFactory.h"
#include "VideoFilter.h"

namespace FFmpegInterop
{
	ref class VideoEffectFactory : public AbstractEffectFactory
	{
		AVCodecContext* InputContext;

	internal:

		VideoEffectFactory(AVCodecContext* input_ctx)
		{
			InputContext = input_ctx;
		}

		IAvEffect^ CreateEffect(IVectorView<AvEffectDefinition^>^ definitions) override
		{
			return nullptr;
		}
	};
}