#pragma once
#include "FrameGrabber.g.h"
#include "FFmpegMediaSource.h"
#include "UncompressedVideoSampleProvider.h"
#include <NativeBuffer.h>

using namespace NativeBuffer;
// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropXWinUI::implementation
{
	struct FrameGrabber : FrameGrabberT<FrameGrabber>
	{
		FrameGrabber() = default;

		static Windows::Foundation::IAsyncOperation<FFmpegInteropXWinUI::FrameGrabber> CreateFromStreamAsync(Windows::Storage::Streams::IRandomAccessStream stream);
		static Windows::Foundation::IAsyncOperation<FFmpegInteropXWinUI::FrameGrabber> CreateFromUriAsync(hstring uri);
		Windows::Foundation::TimeSpan Duration();
		int32_t DecodePixelWidth();
		void DecodePixelWidth(int32_t value);
		int32_t DecodePixelHeight();
		void DecodePixelHeight(int32_t value);
		FFmpegInteropXWinUI::VideoStreamInfo CurrentVideoStream();
		Windows::Foundation::IAsyncOperation<FFmpegInteropXWinUI::VideoFrame> ExtractVideoFrameAsync(Windows::Foundation::TimeSpan position, bool exactSeek, int32_t maxFrameSkip, Windows::Storage::Streams::IBuffer targetBuffer);
		Windows::Foundation::IAsyncOperation<FFmpegInteropXWinUI::VideoFrame> ExtractNextVideoFrameAsync(Windows::Storage::Streams::IBuffer targetBuffer);
		Windows::Foundation::IAsyncOperation<FFmpegInteropXWinUI::VideoFrame> ExtractVideoFrameAsync(Windows::Foundation::TimeSpan position, bool exactSeek, int32_t maxFrameSkip);
		Windows::Foundation::IAsyncOperation<FFmpegInteropXWinUI::VideoFrame> ExtractVideoFrameAsync(Windows::Foundation::TimeSpan position, bool exactSeek);
		Windows::Foundation::IAsyncOperation<FFmpegInteropXWinUI::VideoFrame> ExtractVideoFrameAsync(Windows::Foundation::TimeSpan position);
		Windows::Foundation::IAsyncOperation<FFmpegInteropXWinUI::VideoFrame> ExtractNextVideoFrameAsync();
		void Close();

	public:
		winrt::com_ptr<winrt::FFmpegInteropXWinUI::implementation::FFmpegMediaSource> interopMSS = { nullptr };
		winrt::Windows::Media::MediaProperties::MediaRatio pixelAspectRatio = { nullptr };
		int width;
		int height;
		

		FrameGrabber(winrt::com_ptr<winrt::FFmpegInteropXWinUI::implementation::FFmpegMediaSource> interopMSS)
		{
			this->interopMSS = interopMSS;
		}

	private:

		int decodePixelWidth;
		int decodePixelHeight;

		void PrepareDecoding(winrt::Windows::Storage::Streams::IBuffer targetBuffer)
		{
			// the IBuffer from WriteableBitmap can only be accessed on UI thread
			// so we need to check it and get its pointer here already

			auto sampleProvider = std::static_pointer_cast<FFmpegInteropX::UncompressedVideoSampleProvider>(interopMSS->VideoSampleProvider());
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
				if (length != width * height * 4)
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
namespace winrt::FFmpegInteropXWinUI::factory_implementation
{
	struct FrameGrabber : FrameGrabberT<FrameGrabber, implementation::FrameGrabber>
	{
	};
}
