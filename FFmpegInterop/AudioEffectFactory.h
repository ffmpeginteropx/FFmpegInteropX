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

		IAvEffect^ CreateEffect(IVectorView<AvEffectDefinition^>^ definitions) override
		{
			int numChannels = AvCodecContextHelpers::GetNBChannels(InputContext);
			auto channel_layout = AvCodecContextHelpers::GetChannelLayout(InputContext, numChannels);
			AudioFilter^ filter = ref new AudioFilter(InputContext,channel_layout, numChannels);
			auto hr = filter ? S_OK : E_OUTOFMEMORY;
			if (SUCCEEDED(hr))
			{
				hr = filter->AllocResources(definitions);
			}
			if (SUCCEEDED(hr))
			{
				return filter;
			}
			else
			{
				return nullptr;
			}
		}
	};
}