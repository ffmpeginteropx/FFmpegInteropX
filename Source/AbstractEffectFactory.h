#pragma once
#include "IAvEffect.h"

namespace FFmpegInteropX {
    ref class AbstractEffectFactory abstract
    {
    internal:
        virtual IAvEffect^ CreateEffect(String^ definitions) abstract;
    };
}


