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
			auto filter = ref new VideoFilter(InputContext);

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