#pragma once
#include "IAvEffect.h"
#include "AbstractEffectFactory.h"
#include <mutex>

extern "C"
{
#include <libavformat/avformat.h>
}


using namespace winrt::Windows::Foundation::Collections;

class UncompressedFrameProvider sealed
{
    std::shared_ptr<IAvEffect> filter;
    AVFormatContext* m_pAvFormatCtx = NULL;
    AVCodecContext* m_pAvCodecCtx = NULL;
    std::shared_ptr<AbstractEffectFactory> m_effectFactory;
    winrt::hstring pendingEffects{};

public:
    UncompressedFrameProvider(AVFormatContext* p_pAvFormatCtx, AVCodecContext* p_pAvCodecCtx, std::shared_ptr<AbstractEffectFactory> p_effectFactory)
    {
        m_pAvCodecCtx = p_pAvCodecCtx;
        m_pAvFormatCtx = p_pAvFormatCtx;
        m_effectFactory = p_effectFactory;
    }

    void UpdateCodecContext(AVCodecContext* avCodecCtx)
    {
        m_pAvCodecCtx = avCodecCtx;
    }

    void UpdateFilter(winrt::hstring effects)
    {
        if (!effects.empty())
        {
            pendingEffects = effects;
        }
        else
        {
            pendingEffects.clear();
            filter = nullptr;
        }
    }

    void DisableFilter()
    {
        pendingEffects.clear();
        filter = nullptr;
    }

    HRESULT GetFrame(AVFrame** avFrame)
    {
        HRESULT hr = S_OK;

        if (!pendingEffects.empty())
        {
            if (pendingEffects.size() > 0)
            {
                filter = m_effectFactory->CreateEffect(pendingEffects);
            }
            else
            {
                filter = nullptr;
            }

            pendingEffects.clear();
        }

        if (filter)
        {
            hr = GetFrameFromFilter(avFrame);
        }
        else
        {
            hr = GetFrameFromCodec(avFrame);
        }

        return hr;
    }

    HRESULT GetFrameFromFilter(AVFrame** avFrame)
    {
        HRESULT hr = S_OK;

        // use polling loop and push frames only if needed
        while (SUCCEEDED(hr))
        {
            hr = filter->GetFrame(*avFrame);
            if (hr == AVERROR(EAGAIN))
            {
                // filter requires next source frame.
                // get from codec and feed to filter graph.

                hr = GetFrameFromCodec(avFrame);
                if (SUCCEEDED(hr) && (*avFrame)->format == AV_PIX_FMT_D3D11)
                {
                    // this is a hardware frame, replace it with a software copy
                    hr = ConvertHwToSwFrame(avFrame);
                }

                if (SUCCEEDED(hr))
                {
                    hr = filter->AddFrame(*avFrame);
                    if (FAILED(hr))
                    {
                        // add frame failed. clear filter to prevent crashes.
                        filter = nullptr;
                    }
                }
                else if (hr == AVERROR_EOF)
                {
                    // feed NULL packet to filter to enter draining mode on EOF
                    hr = filter->AddFrame(NULL);
                    if (FAILED(hr))
                    {
                        // add frame failed. clear filter to prevent crashes.
                        filter = nullptr;
                    }
                }
            }
            else
            {
                // filter has either success or failure
                break;
            }
        }

        return hr;
    }

    HRESULT GetFrameFromCodec(AVFrame** avFrame)
    {
        HRESULT hr = avcodec_receive_frame(m_pAvCodecCtx, *avFrame);
        return hr;
    }

    HRESULT ConvertHwToSwFrame(AVFrame** avFrame)
    {
        HRESULT hr = S_OK;
        AVFrame* swFrame = av_frame_alloc();
        if (!swFrame)
        {
            hr = E_OUTOFMEMORY;
        }
        if (SUCCEEDED(hr))
        {
            AVPixelFormat* formats;
            av_hwframe_transfer_get_formats((*avFrame)->hw_frames_ctx, AVHWFrameTransferDirection::AV_HWFRAME_TRANSFER_DIRECTION_FROM, &formats, 0);
            swFrame->format = formats[0];
            hr = av_hwframe_transfer_data(swFrame, *avFrame, 0);
        }
        if (SUCCEEDED(hr))
        {
            av_frame_copy_props(swFrame, *avFrame);
        }
        if (SUCCEEDED(hr))
        {
            av_frame_free(avFrame);
            *avFrame = swFrame;
        }
        else
        {
            av_frame_free(&swFrame);
        }

        return hr;
    }
};

