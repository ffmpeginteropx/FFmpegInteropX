#include "pch.h"
#include "VideoFrame.h"
#include "VideoFrame.g.cpp"

namespace winrt::FFmpegInteropX::implementation
{
    VideoFrame::VideoFrame(Windows::Storage::Streams::IBuffer const& pixelData, uint32_t width, uint32_t height, Windows::Media::MediaProperties::MediaRatio const& pixelAspectRatio, Windows::Foundation::TimeSpan const& timestamp)
    {
        this->pixelData = pixelData;
        this->pixelWidth = width;
        this->pixelHeight = height;
        this->pixelAspectRatio = pixelAspectRatio;
        if (pixelAspectRatio != nullptr &&
            pixelAspectRatio.Numerator() > 0 &&
            pixelAspectRatio.Denominator() > 0 &&
            pixelAspectRatio.Numerator() != pixelAspectRatio.Denominator())
        {
            hasNonSquarePixels = true;
        }
        this->timestamp = timestamp;
    }

    VideoFrame::~VideoFrame()
    {
        Close();
    }

    void VideoFrame::Close()
    {
        pixelData = { nullptr };
    }

    Windows::Storage::Streams::IBuffer VideoFrame::PixelData()
    {
        return pixelData;
    }

    uint32_t VideoFrame::PixelWidth()
    {
        return pixelWidth;
    }

    uint32_t VideoFrame::PixelHeight()
    {
        return pixelHeight;
    }

    Windows::Media::MediaProperties::MediaRatio VideoFrame::PixelAspectRatio()
    {
        return pixelAspectRatio;
    }

    Windows::Foundation::TimeSpan VideoFrame::Timestamp()
    {
        return timestamp;
    }

    Windows::Foundation::IAsyncAction VideoFrame::EncodeAsBmpAsync(Windows::Storage::Streams::IRandomAccessStream stream)
    {
        return this->Encode(stream, BitmapEncoder::BmpEncoderId());
    }

    Windows::Foundation::IAsyncAction VideoFrame::EncodeAsJpegAsync(Windows::Storage::Streams::IRandomAccessStream stream)
    {
        return this->Encode(stream, BitmapEncoder::JpegEncoderId());
    }

    Windows::Foundation::IAsyncAction VideoFrame::EncodeAsPngAsync(Windows::Storage::Streams::IRandomAccessStream stream)
    {
        return this->Encode(stream, BitmapEncoder::PngEncoderId());
    }

    uint32_t VideoFrame::DisplayWidth()
    {
        if (hasNonSquarePixels)
        {
            if (pixelAspectRatio.Numerator() > pixelAspectRatio.Denominator())
            {
                return (unsigned int)round(((double)pixelAspectRatio.Numerator() / pixelAspectRatio.Denominator()) * pixelWidth);
            }
        }
        return pixelWidth;
    }

    uint32_t VideoFrame::DisplayHeight()
    {
        if (hasNonSquarePixels)
        {
            if (pixelAspectRatio.Numerator() < pixelAspectRatio.Denominator())
            {
                return (unsigned int)round(((double)pixelAspectRatio.Denominator() / pixelAspectRatio.Numerator()) * pixelHeight);
            }
        }
        return pixelHeight;
    }

    double VideoFrame::DisplayAspectRatio()
    {
        double result = (double)pixelWidth / pixelHeight;
        if (hasNonSquarePixels)
        {
            return result * pixelAspectRatio.Numerator() / pixelAspectRatio.Denominator();
        }
        return result;
    }
}
