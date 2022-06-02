#pragma once
#include "IAvEffect.h"

namespace FFmpegInteropX {
	class AbstractEffectFactory abstract
	{
	public:		
		virtual std::shared_ptr<IAvEffect> CreateEffect(winrt::hstring definitions) abstract;
	};
}


