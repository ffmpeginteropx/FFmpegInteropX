#pragma once
#include "AbstractEffectFactory.h"
#include "AudioFilter.h"
#include "AvCodecContextHelpers.h"

namespace FFmpegInteropX
{
	ref class AudioEffectFactory : public AbstractEffectFactory
	{
		AVCodecContext* InputContext;	

	internal:

		AudioEffectFactory(AVCodecContext* input_ctx)
		{
			InputContext = input_ctx;
		}

		IAvEffect^ CreateEffect(String^ filterDefinition) override
		{
			return ref new AudioFilter(InputContext, filterDefinition);
		}
	};
}