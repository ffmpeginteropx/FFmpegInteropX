#pragma once
#include "AbstractEffectFactory.h"
#include "AudioFilter.h"
#include "AvCodecContextHelpers.h"

namespace FFmpegInterop
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
			int numChannels = AvCodecContextHelpers::GetNBChannels(InputContext);
			auto channel_layout = AvCodecContextHelpers::GetChannelLayout(InputContext, numChannels);
			return ref new AudioFilter(InputContext,channel_layout, numChannels, filterDefinition);
		}
	};
}