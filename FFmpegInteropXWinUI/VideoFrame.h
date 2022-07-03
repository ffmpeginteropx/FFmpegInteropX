#pragma once
#include "VideoFrame.g.h"
#include "NativeBuffer.h"
#include <winrt/Windows.Graphics.Imaging.h>
// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");
using namespace winrt::Windows::Graphics::Imaging;
using namespace NativeBuffer;

namespace winrt::FFmpegInteropXWinUI::implementation
{
    struct VideoFrame : VideoFrameT<VideoFrame>
    {
        VideoFrame() = default;

        VideoFrame(Windows::Storage::Streams::IBuffer const& pixelData, uint32_t width, uint32_t height, Windows::Media::MediaProperties::MediaRatio const& pixelAspectRatio, Windows::Foundation::TimeSpan const& timestamp);
        Windows::Storage::Streams::IBuffer PixelData();
        uint32_t PixelWidth();
        uint32_t PixelHeight();
        Windows::Media::MediaProperties::MediaRatio PixelAspectRatio();
        Windows::Foundation::TimeSpan Timestamp();
        Windows::Foundation::IAsyncAction EncodeAsBmpAsync(Windows::Storage::Streams::IRandomAccessStream stream);
        Windows::Foundation::IAsyncAction EncodeAsJpegAsync(Windows::Storage::Streams::IRandomAccessStream stream);
        Windows::Foundation::IAsyncAction EncodeAsPngAsync(Windows::Storage::Streams::IRandomAccessStream stream);
        uint32_t DisplayWidth();
        uint32_t DisplayHeight();
        double DisplayAspectRatio();

    private:
        winrt::Windows::Storage::Streams::IBuffer pixelData = { nullptr };
        unsigned int pixelWidth = 0;
        unsigned int pixelHeight = 0;
        winrt::Windows::Foundation::TimeSpan timestamp{};
        winrt::Windows::Media::MediaProperties::MediaRatio pixelAspectRatio = { nullptr };
        bool hasNonSquarePixels = false;

        winrt::Windows::Foundation::IAsyncAction Encode(winrt::Windows::Storage::Streams::IRandomAccessStream stream, winrt::guid encoderGuid)
        {
            // Query the IBufferByteAccess interface.  
            winrt::com_ptr<IBufferByteAccess> bufferByteAccess;
            (pixelData).as(IID_PPV_ARGS(&bufferByteAccess));

            // Retrieve the buffer data.  
            byte* pixels = nullptr;
            bufferByteAccess->Buffer(&pixels);
            auto length = pixelData.Length();

            auto encoderValue = co_await winrt::Windows::Graphics::Imaging::BitmapEncoder::CreateAsync(encoderGuid, stream);

            if (hasNonSquarePixels)
            {
                encoderValue.BitmapTransform().ScaledWidth(DisplayWidth());
                encoderValue.BitmapTransform().ScaledHeight(DisplayHeight());
                encoderValue.BitmapTransform().InterpolationMode(BitmapInterpolationMode::Linear);
            }

            encoderValue.SetPixelData(
                BitmapPixelFormat::Bgra8,
                BitmapAlphaMode::Ignore,
                pixelWidth,
                pixelHeight,
                72,
                72,
                std::vector<byte>(pixels, pixels + length));

            co_await encoderValue.FlushAsync();
        }

    };
}
namespace winrt::FFmpegInteropXWinUI::factory_implementation
{
    struct VideoFrame : VideoFrameT<VideoFrame, implementation::VideoFrame>
    {
    };
}
