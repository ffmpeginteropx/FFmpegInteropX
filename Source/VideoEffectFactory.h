#pragma once
#include "AbstractEffectFactory.h"
#include "VideoFilter.h"

namespace FFmpegInteropX
{
    ref class VideoEffectFactory : public AbstractEffectFactory
    {
        AVCodecContext* inputContext;
        AVStream* inputStream;

    internal:

        VideoEffectFactory(AVCodecContext* input_ctx, AVStream* inputStream)
        {
            this->inputContext = input_ctx;
            this->inputStream = inputStream;
        }

        IAvEffect^ CreateEffect(String^ filterDefinition) override
        {
            /*Since video often requires HW acceleration for acceptable framerates,
            we used IBasicVodeoEffect to implement video filters,
            which supports hardware acceleration.
            FFmpeg filters only work with AVFrame, which contains raw data,
            hence it is unsuitable for real time playback. There could be scenarios
            in which the extensive video filer library of FFmpeg could be used (i.e transcoding).*/

            return ref new VideoFilter(inputContext, inputStream, filterDefinition);
        }
    };
}