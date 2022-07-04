#pragma once
#include "AbstractEffectFactory.h"
#include "AudioFilter.h"
#include "AvCodecContextHelpers.h"

namespace FFmpegInteropX
{
    class AudioEffectFactory : public AbstractEffectFactory
    {
        AVCodecContext* InputContext;

    public:

        AudioEffectFactory(AVCodecContext* input_ctx)
        {
            InputContext = input_ctx;
        }

        std::shared_ptr<IAvEffect> CreateEffect(winrt::hstring const& filterDefinition) override
        {
            return std::shared_ptr<AudioFilter>(new AudioFilter(InputContext, filterDefinition));
        }
    };
}