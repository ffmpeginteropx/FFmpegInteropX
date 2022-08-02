#pragma once
#include "pch.h"
#include "AbstractEffectFactory.h"
#include "VideoFilter.h"


class VideoEffectFactory : public AbstractEffectFactory
{
    AVCodecContext* inputContext = NULL;
    AVStream* inputStream = NULL;

public:

    VideoEffectFactory(AVCodecContext* input_ctx, AVStream* inputStream)
    {
        this->inputContext = input_ctx;
        this->inputStream = inputStream;
    }

    std::shared_ptr<IAvEffect> CreateEffect(winrt::hstring const& filterDefinition) override
    {
        /*Since video often requires HW acceleration for acceptable framerates,
        we used IBasicVodeoEffect to implement video filters,
        which supports hardware acceleration.
        FFmpeg filters only work with AVFrame, which contains raw data,
        hence it is unsuitable for real time playback. There could be scenarios
        in which the extensive video filer library of FFmpeg could be used (i.e transcoding).*/

        return std::shared_ptr<VideoFilter>(new VideoFilter(inputContext, inputStream, filterDefinition));
    }
};
