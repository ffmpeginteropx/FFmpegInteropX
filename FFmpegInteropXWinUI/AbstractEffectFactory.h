#pragma once
#include "pch.h"
#include "IAvEffect.h"

namespace FFmpegInteropX {
	class AbstractEffectFactory abstract
	{
	public:		
		virtual std::shared_ptr<IAvEffect> CreateEffect(winrt::hstring const& definitions) abstract;
	};
}


