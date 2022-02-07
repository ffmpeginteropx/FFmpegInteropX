#pragma once
#include "AvEffectDefinition.h"
#include "IAvEffect.h"

using namespace Windows::Foundation::Collections;


namespace FFmpegInteropX {
	ref class AbstractEffectFactory abstract
	{
	internal:
		virtual IAvEffect^ CreateEffect(String^ definitions) abstract;
	};
}


