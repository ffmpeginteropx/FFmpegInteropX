#pragma once
extern "C"
{
#include <libavformat/avformat.h>
}

class IAvFilter
{
public:
    virtual	~IAvFilter() {}

    virtual HRESULT AddFrame(AVFrame* frame) = 0;

    virtual HRESULT GetFrame(AVFrame* frame) = 0;

    virtual bool IsInitialized() = 0;
};
