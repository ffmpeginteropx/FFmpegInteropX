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
			/*Since video often requires HW acceleration for acceptable framerates,
			we used IBasicVodeoEffect to implement video filters,
			which supports hardware acceleration.
			FFmpeg filters only work with AVFrame, which contains raw data,
			hence it is unsuitable for real time playback. There could be scenarios 
			in which the extensive video filer library of FFmpeg could be used (i.e transcoding).*/

			VideoFilter^ filter = ref new VideoFilter(InputContext);
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

			return nullptr;
		}
	};
}