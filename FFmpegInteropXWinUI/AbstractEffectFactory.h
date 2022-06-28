#pragma once
#include "pch.h"
#include "IAvEffect.h"

namespace FFmpegInteropX {
	class AbstractEffectFactory
	{
	public:		
        virtual std::shared_ptr<IAvEffect> CreateEffect(winrt::hstring const& definitions) = 0;
	};
}


