#pragma once

#include <ppl.h>

#include "FrameGrabber.h"

namespace FFmpegInterop
{
	using namespace Concurrency;
	using namespace Platform;
	using namespace Platform::Collections;
	using namespace Windows::Foundation;
	using namespace Windows::Foundation::Collections;
	using namespace Windows::Media;
	using namespace Windows::Media::MediaProperties;
	using namespace Windows::Storage;
	using namespace Windows::Storage::Streams;

	public ref class ImageListSource sealed
	{
	public:
		static IAsyncOperation<ImageListSource^>^ CreateFromFilesAsync(IVectorView<StorageFile^>^ files, double framesPerSecond)
		{
			return create_async([files, framesPerSecond]() -> task<ImageListSource^>
				{
					auto source = ref new ImageListSource(files, framesPerSecond);
					co_await source->InitializeAsync();
					co_return source;
				});
		}

		property MediaStreamSource^ MediaStreamSource { Windows::Media::Core::MediaStreamSource^ get() { return mss; } };

	private:

		ImageListSource(IVectorView<StorageFile^> ^ files, double framesPerSecond)
		{
			this->files = files;
			this->framesPerSecond = framesPerSecond;
		}


		task<void> InitializeAsync()
		{
			firstFrame = co_await GetCurrentFrameAsync();
			if (!firstFrame)
			{
				throw ref new Exception(E_FAIL);
			}

			encodingProperties = VideoEncodingProperties::CreateUncompressed(
				MediaEncodingSubtypes::Argb32, firstFrame->PixelWidth, firstFrame->PixelHeight);

			encodingProperties->FrameRate->Numerator = framesPerSecond*100000;
			encodingProperties->FrameRate->Denominator = 100000;

			encodingProperties->PixelAspectRatio->Numerator = firstFrame->PixelAspectRatio->Numerator;
			encodingProperties->PixelAspectRatio->Denominator = firstFrame->PixelAspectRatio->Denominator;

			streamDescriptor = ref new VideoStreamDescriptor(encodingProperties);

			mss = ref new Windows::Media::Core::MediaStreamSource(streamDescriptor);
			mss->Duration = TimeSpan{ (long long)(((double)files->Size / framesPerSecond) * 10000000) };
			mss->CanSeek = true;
			mss->BufferTime = TimeSpan{ 0 };
			
			mss->Starting += ref new Windows::Foundation::TypedEventHandler<Windows::Media::Core::MediaStreamSource^, Windows::Media::Core::MediaStreamSourceStartingEventArgs^>(this, &FFmpegInterop::ImageListSource::OnStarting);
			mss->SampleRequested += ref new Windows::Foundation::TypedEventHandler<Windows::Media::Core::MediaStreamSource^, Windows::Media::Core::MediaStreamSourceSampleRequestedEventArgs^>(this, &FFmpegInterop::ImageListSource::OnSampleRequested);
		}

		void OnStarting(Windows::Media::Core::MediaStreamSource^ sender, Windows::Media::Core::MediaStreamSourceStartingEventArgs^ args)
		{
			if (args->Request->StartPosition)
			{
				index = (int)(args->Request->StartPosition->Value.Duration * framesPerSecond / 10000000.0);
			}
		}
		
		void OnSampleRequested(Windows::Media::Core::MediaStreamSource^ sender, Windows::Media::Core::MediaStreamSourceSampleRequestedEventArgs^ args)
		{
			try
			{
				if (index == 0 && firstFrame)
				{
					args->Request->Sample = CreateSampleFromBuffer(firstFrame->PixelData);
					firstFrame = nullptr;
				}
				else
				{
					auto frame = GetCurrentFrameAsync().get();
					if (frame)
					{
						args->Request->Sample = CreateSampleFromBuffer(frame->PixelData);
					}
				}
			}
			catch (...)
			{
			}
			index++;
		}

		MediaStreamSample^ CreateSampleFromBuffer(IBuffer^ buffer)
		{
			auto duration = TimeSpan{ (long long)(10000000.0 / framesPerSecond) };
			auto timestamp = TimeSpan{ duration.Duration * index };
			auto sample = MediaStreamSample::CreateFromBuffer(buffer, timestamp);
			sample->Duration = duration;
			return sample;
		}


		task<VideoFrame^> GetCurrentFrameAsync()
		{
			if (index < files->Size)
			{
				try
				{
					auto file = files->GetAt(index);
					auto stream = co_await file->OpenReadAsync();
					auto frameGrabber = co_await FrameGrabber::CreateFromStreamAsync(stream);
					auto frame = co_await frameGrabber->ExtractNextVideoFrameAsync();
					co_return frame;
				}
				catch (...)
				{
				}
			}

			co_return nullptr;
		}

		unsigned int index;
		VideoFrame^ firstFrame;
		IVectorView<StorageFile^>^ files;
		double framesPerSecond;
		unsigned int pixelWidth;
		unsigned int pixelHeight;
		Windows::Media::Core::MediaStreamSource^ mss;
		VideoStreamDescriptor^ streamDescriptor;
		VideoEncodingProperties^ encodingProperties;
	};
}


