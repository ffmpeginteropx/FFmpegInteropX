#pragma once
#include "FrameGrabber.g.h"
#include "FFmpegMediaSource.h"
#include "UncompressedVideoSampleProvider.h"
#include "NativeBuffer.h"

using namespace NativeBuffer;
// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropX::implementation
{
    struct FrameGrabber : FrameGrabberT<FrameGrabber>
    {
        FrameGrabber(winrt::com_ptr<winrt::FFmpegInteropX::implementation::FFmpegMediaSource> interopMSS)
        {
            this->interopMSS = interopMSS;
        }

        /// <summary>Creates a new FrameGrabber from the specified stream.</summary>
        static Windows::Foundation::IAsyncOperation<FFmpegInteropX::FrameGrabber> CreateFromStreamAsync(Windows::Storage::Streams::IRandomAccessStream stream);

        /// <summary>Creates a new FrameGrabber from the specified uri.</summary>
        static Windows::Foundation::IAsyncOperation<FFmpegInteropX::FrameGrabber> CreateFromUriAsync(hstring const& uri);

        /// <summary>The duration of the video stream.</summary>
        Windows::Foundation::TimeSpan Duration();

        /// <summary>Gets or sets the decode pixel width.</summary>
        int32_t DecodePixelWidth();
        void DecodePixelWidth(int32_t value);

        /// <summary>Gets or sets the decode pixel height.</summary>
        int32_t DecodePixelHeight();
        void DecodePixelHeight(int32_t value);

        /// <summary>Gets the current video stream information.</summary>
        FFmpegInteropX::VideoStreamInfo CurrentVideoStream();

        /// <summary>Extracts a video frame at the specififed position.</summary>
        /// <param name="position">The position of the requested frame.</param>
        /// <param name="exactSeek">If set to false, this will decode the closest previous key frame, which is faster but not as precise.</param>
        /// <param name="maxFrameSkip">If exactSeek=true, this limits the number of frames to decode after the key frame.</param>
        /// <param name="targetBuffer">The target buffer which shall contain the decoded pixel data.</param>
        /// <remarks>The IAsyncOperation result supports cancellation, so long running frame requests (exactSeek=true) can be interrupted.</remarks>
        Windows::Foundation::IAsyncOperation<FFmpegInteropX::VideoFrame> ExtractVideoFrameAsync(Windows::Foundation::TimeSpan position, bool exactSeek, int32_t maxFrameSkip, Windows::Storage::Streams::IBuffer targetBuffer);

        /// <summary>Extracts the next consecutive video frame in the file. Returns <c>null</c> at end of stream.</summary>
        /// <param name="targetBuffer">The target buffer which shall contain the decoded pixel data.</param>
        Windows::Foundation::IAsyncOperation<FFmpegInteropX::VideoFrame> ExtractNextVideoFrameAsync(Windows::Storage::Streams::IBuffer targetBuffer);

        /// <summary>Extracts a video frame at the specififed position.</summary>
        /// <param name="position">The position of the requested frame.</param>
        /// <param name="exactSeek">If set to false, this will decode the closest previous key frame, which is faster but not as precise.</param>
        /// <param name="maxFrameSkip">If exactSeek=true, this limits the number of frames to decode after the key frame.</param>
        /// <remarks>The IAsyncOperation result supports cancellation, so long running frame requests (exactSeek=true) can be interrupted.</remarks>
        Windows::Foundation::IAsyncOperation<FFmpegInteropX::VideoFrame> ExtractVideoFrameAsync(Windows::Foundation::TimeSpan position, bool exactSeek, int32_t maxFrameSkip);

        /// <summary>Extracts a video frame at the specififed position.</summary>
        /// <param name="position">The position of the requested frame.</param>
        /// <param name="exactSeek">If set to false, this will decode the closest previous key frame, which is faster but not as precise.</param>
        Windows::Foundation::IAsyncOperation<FFmpegInteropX::VideoFrame> ExtractVideoFrameAsync(Windows::Foundation::TimeSpan position, bool exactSeek);

        /// <summary>Extracts a video frame at the specififed position.</summary>
        /// <param name="position">The position of the requested frame.</param>
        /// <remarks>The IAsyncOperation result supports cancellation, so long running frame requests (exactSeek=true) can be interrupted.</remarks>
        Windows::Foundation::IAsyncOperation<FFmpegInteropX::VideoFrame> ExtractVideoFrameAsync(Windows::Foundation::TimeSpan position);

        /// <summary>Extracts the next consecutive video frame in the file. Returns <c>null</c> at end of stream.</summary>
        Windows::Foundation::IAsyncOperation<FFmpegInteropX::VideoFrame> ExtractNextVideoFrameAsync();

        void Close();

    private:
        winrt::com_ptr<winrt::FFmpegInteropX::implementation::FFmpegMediaSource> interopMSS = { nullptr };
        winrt::Windows::Media::MediaProperties::MediaRatio pixelAspectRatio = { nullptr };
        int width = 0;
        int height = 0;
        int decodePixelWidth = 0;
        int decodePixelHeight = 0;

        void PrepareDecoding(winrt::Windows::Storage::Streams::IBuffer const& targetBuffer)
        {
            // the IBuffer from WriteableBitmap can only be accessed on UI thread
            // so we need to check it and get its pointer here already

            auto sampleProvider = std::static_pointer_cast<UncompressedVideoSampleProvider>(interopMSS->VideoSampleProvider());
            auto streamDescriptor = (interopMSS->VideoSampleProvider()->StreamDescriptor().as<VideoStreamDescriptor>());
            pixelAspectRatio = streamDescriptor.EncodingProperties().PixelAspectRatio();
            if (DecodePixelWidth() > 0 &&
                DecodePixelHeight() > 0)
            {
                sampleProvider->TargetWidth = DecodePixelWidth();
                sampleProvider->TargetHeight = DecodePixelHeight();
                pixelAspectRatio.Numerator(1);
                pixelAspectRatio.Denominator(1);
            }
            else
            {
                // lock frame size - no dynamic size changes during frame grabbing!
                sampleProvider->TargetWidth = sampleProvider->VideoInfo().PixelWidth();
                sampleProvider->TargetHeight = sampleProvider->VideoInfo().PixelHeight();
            }
            width = sampleProvider->TargetWidth;
            height = sampleProvider->TargetHeight;

            BYTE* pixels = nullptr;
            if (targetBuffer)
            {
                auto length = targetBuffer.Length();
                if (length != (uint32_t)width * height * 4)
                {
                    throw_hresult(E_INVALIDARG);
                }

                // Query the IBufferByteAccess interface.  
                winrt::com_ptr<IBufferByteAccess> bufferByteAccess;
                targetBuffer.as(IID_PPV_ARGS(&bufferByteAccess));

                // Retrieve the buffer data.  
                bufferByteAccess->Buffer(&pixels);
            }
            sampleProvider->TargetBuffer = pixels;
        }

    };
}
namespace winrt::FFmpegInteropX::factory_implementation
{
    struct FrameGrabber : FrameGrabberT<FrameGrabber, implementation::FrameGrabber>
    {
    };
}
