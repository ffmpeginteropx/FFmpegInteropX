#pragma once
extern "C"
{
#include <libavformat/avformat.h>
}

class IAvEffect
{
public:
    virtual	~IAvEffect() {}

    virtual HRESULT AddFrame(AVFrame* frame) = 0;

    virtual HRESULT GetFrame(AVFrame* frame) = 0;
};
