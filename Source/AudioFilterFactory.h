#pragma once
#include "AvFilterFactoryBase.h"
#include "AudioFilter.h"
#include "AvCodecContextHelpers.h"


class AudioFilterFactory : public AvFilterFactoryBase
{
    AVCodecContext* InputContext;

public:

    AudioFilterFactory(AVCodecContext* input_ctx)
    {
        InputContext = input_ctx;
    }

    std::shared_ptr<IAvFilter> CreateEffect(winrt::hstring const& filterDefinition) override
    {
        return std::shared_ptr<AudioFilter>(new AudioFilter(InputContext, filterDefinition));
    }
};
