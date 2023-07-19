#pragma once
#include "pch.h"
#include "IAvEffect.h"

class AbstractEffectFactory
{
public:
    virtual std::shared_ptr<IAvFilter> CreateEffect(winrt::hstring const& definitions) = 0;
};


