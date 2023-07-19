#pragma once
#include "pch.h"
#include "IAvFilter.h"

class AvFilterFactoryBase
{
public:
    virtual std::shared_ptr<IAvFilter> CreateEffect(winrt::hstring const& definitions) = 0;
};


