#pragma once
#include "winrt/base.h"
#include "FilterCommandResult.h"

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

    virtual FilterCommandResult SendCommand(winrt::hstring target, winrt::hstring command, winrt::hstring arguments) = 0;

    virtual bool IsInitialized() = 0;
};
