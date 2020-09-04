#pragma once
#include "FFmpegInteropMSS.h"
#include "FFmpegInteropConfig.h"
#include "UncompressedVideoSampleProvider.h"
#include <ppl.h>
#include <robuffer.h>


namespace FFmpegInterop {

	using namespace concurrency;
	using namespace Windows::Storage::Streams;

	/// <summary>Supports grabbing of video frames from a file.</summary>
	public ref class FrameGrabber sealed
	{

		FFmpegInteropMSS^ interopMSS;

	internal:
		FrameGrabber(FFmpegInteropMSS^ interopMSS) {
			this->interopMSS = interopMSS;
		}

	public:

		virtual ~FrameGrabber() {
			if (interopMSS)
				delete interopMSS;
		}

		/// <summary>The duration of the video stream.</summary>
		property TimeSpan Duration
		{
			TimeSpan get()
			{
				return interopMSS->Duration;
			}
		}

		/// <summary>Gets or sets the decode pixel width.</summary>
		property int DecodePixelWidth;

		/// <summary>Gets or sets the decode pixel height.</summary>
		property int DecodePixelHeight;

		/// <summary>Gets the current video stream information.</summary>
		property VideoStreamInfo^ CurrentVideoStream
		{
			VideoStreamInfo^ get() { return interopMSS->VideoStream; }
		}

		/// <summary>Creates a new FrameGrabber from the specified stream.</summary>
		static IAsyncOperation<FrameGrabber^>^ CreateFromStreamAsync(IRandomAccessStream^ stream)
		{
			return create_async([stream]
			{
				FFmpegInteropConfig^ config = ref new FFmpegInteropConfig();
				config->IsFrameGrabber = true;
				config->VideoDecoderMode = VideoDecoderMode::ForceFFmpegSoftwareDecoder;

				auto result = FFmpegInteropMSS::CreateFromStream(stream, config, nullptr, nullptr);
				if (result == nullptr)
				{
					throw ref new Exception(E_FAIL, "Could not create MediaStreamSource.");
				}
				if (result->VideoStream == nullptr)
				{
					throw ref new Exception(E_FAIL, "No video stream found in file (or no suitable decoder available).");
				}
				if (result->VideoSampleProvider == nullptr)
				{
					throw ref new Exception(E_FAIL, "No video stream found in file (or no suitable decoder available).");
				}
				return ref new FrameGrabber(result);
			});
		}

		/// <summary>Extracts a video frame at the specififed position.</summary>
		/// <param name="position">The position of the requested frame.</param>
		/// <param name="exactSeek">If set to false, this will decode the closest previous key frame, which is faster but not as precise.</param>
		/// <param name="maxFrameSkip">If exactSeek=true, this limits the number of frames to decode after the key frame.</param>
		/// <param name="targetBuffer">The target buffer which shall contain the decoded pixel data.</param>
		/// <remarks>The IAsyncOperation result supports cancellation, so long running frame requests (exactSeek=true) can be interrupted.</remarks>
		IAsyncOperation<VideoFrame^>^ ExtractVideoFrameAsync(TimeSpan position, bool exactSeek, int maxFrameSkip, IBuffer^ targetBuffer)
		{
			// the IBuffer from WriteableBitmap can only be accessed on UI thread
			// so we need to check it and get its pointer here already

			auto sampleProvider = static_cast<UncompressedVideoSampleProvider^>(interopMSS->VideoSampleProvider);
			auto streamDescriptor = static_cast<VideoStreamDescriptor^>(interopMSS->VideoSampleProvider->StreamDescriptor);
			MediaRatio^ pixelAspectRatio = streamDescriptor->EncodingProperties->PixelAspectRatio;
			if (DecodePixelWidth > 0 &&
				DecodePixelHeight > 0)
			{
				sampleProvider->TargetWidth = DecodePixelWidth;
				sampleProvider->TargetHeight = DecodePixelHeight;
				pixelAspectRatio->Numerator = 1;
				pixelAspectRatio->Denominator = 1;
			}
			auto width = sampleProvider->TargetWidth;
			auto height = sampleProvider->TargetHeight;

			byte* pixels = nullptr;
			if (targetBuffer)
			{
				auto length = targetBuffer->Length;
				if (length != width * height * 4)
				{
					throw ref new InvalidArgumentException();
				}

				// Query the IBufferByteAccess interface.  
				Microsoft::WRL::ComPtr<IBufferByteAccess> bufferByteAccess;
				reinterpret_cast<IInspectable*>(targetBuffer)->QueryInterface(IID_PPV_ARGS(&bufferByteAccess));

				// Retrieve the buffer data.  
				bufferByteAccess->Buffer(&pixels);
			}
			sampleProvider->TargetBuffer = pixels;

			return create_async([this, position, exactSeek, maxFrameSkip, width, height, pixelAspectRatio]
			{
				bool seekSucceeded = false;
				if (interopMSS->Duration.Duration >= position.Duration)
				{
					seekSucceeded = SUCCEEDED(interopMSS->Seek(position));
				}

				int framesSkipped = 0;
				MediaStreamSample^ lastSample = nullptr;
				bool gotSample = true;

				while (true)
				{
					interruption_point();
					
					auto sample = interopMSS->VideoSampleProvider->GetNextSample();
					if (sample == nullptr)
					{
						// if we hit end of stream, use last decoded sample (if any), otherwise fail
						if (lastSample != nullptr)
						{
							sample = lastSample;
							gotSample = false;
						}
						else
						{
							throw ref new Exception(E_FAIL, "Failed to decode video frame, or end of stream.");
						}
					}
					else
					{
						lastSample = sample;
					}

					// if exact seek, continue decoding until we have the right sample
					if (gotSample && exactSeek && seekSucceeded && (position.Duration - sample->Timestamp.Duration > sample->Duration.Duration) &&
						(maxFrameSkip <= 0 || framesSkipped < maxFrameSkip))
					{
						framesSkipped++;
						continue;
					}

					//  make sure we have a clean sample (key frame, no half interlaced frame)
					if (gotSample && !interopMSS->VideoSampleProvider->IsCleanSample &&
						(maxFrameSkip <= 0 || framesSkipped < maxFrameSkip))
					{
						framesSkipped++;
						continue;
					}

					auto result = ref new VideoFrame(sample->Buffer,
						width, height, pixelAspectRatio,
						sample->Timestamp);

					return result;

				}
			});
		}

		/// <summary>Extracts the next consecutive video frame in the file. Returns <c>null</c> at end of stream.</summary>
		/// <param name="targetBuffer">The target buffer which shall contain the decoded pixel data.</param>
		IAsyncOperation<VideoFrame^>^ ExtractNextVideoFrameAsync(IBuffer^ targetBuffer)
		{
			// the IBuffer from WriteableBitmap can only be accessed on UI thread
			// so we need to check it and get its pointer here already

			auto sampleProvider = static_cast<UncompressedVideoSampleProvider^>(interopMSS->VideoSampleProvider);
			auto streamDescriptor = static_cast<VideoStreamDescriptor^>(interopMSS->VideoSampleProvider->StreamDescriptor);
			MediaRatio^ pixelAspectRatio = streamDescriptor->EncodingProperties->PixelAspectRatio;
			if (DecodePixelWidth > 0 &&
				DecodePixelHeight > 0)
			{
				sampleProvider->TargetWidth = DecodePixelWidth;
				sampleProvider->TargetHeight = DecodePixelHeight;
				pixelAspectRatio->Numerator = 1;
				pixelAspectRatio->Denominator = 1;
			}
			auto width = sampleProvider->TargetWidth;
			auto height = sampleProvider->TargetHeight;

			byte* pixels = nullptr;
			if (targetBuffer)
			{
				auto length = targetBuffer->Length;
				if (length != width * height * 4)
				{
					throw ref new InvalidArgumentException();
				}

				// Query the IBufferByteAccess interface.  
				Microsoft::WRL::ComPtr<IBufferByteAccess> bufferByteAccess;
				reinterpret_cast<IInspectable*>(targetBuffer)->QueryInterface(IID_PPV_ARGS(&bufferByteAccess));

				// Retrieve the buffer data.  
				bufferByteAccess->Buffer(&pixels);
			}
			sampleProvider->TargetBuffer = pixels;

			return create_async([this, width, height, pixelAspectRatio]
				{
					auto sample = interopMSS->VideoSampleProvider->GetNextSample();
					VideoFrame^ result = nullptr;

					if (sample)
					{
						result = ref new VideoFrame(sample->Buffer,
							width, height, pixelAspectRatio,
							sample->Timestamp);
					}

					return result;
				});
		}

		/// <summary>Extracts a video frame at the specififed position.</summary>
		/// <param name="position">The position of the requested frame.</param>
		/// <param name="exactSeek">If set to false, this will decode the closest previous key frame, which is faster but not as precise.</param>
		/// <param name="maxFrameSkip">If exactSeek=true, this limits the number of frames to decode after the key frame.</param>
		/// <remarks>The IAsyncOperation result supports cancellation, so long running frame requests (exactSeek=true) can be interrupted.</remarks>
		IAsyncOperation<VideoFrame^>^ ExtractVideoFrameAsync(TimeSpan position, bool exactSeek, int maxFrameSkip) { return ExtractVideoFrameAsync(position, exactSeek, maxFrameSkip, nullptr); };

		/// <summary>Extracts a video frame at the specififed position.</summary>
		/// <param name="position">The position of the requested frame.</param>
		/// <param name="exactSeek">If set to false, this will decode the closest previous key frame, which is faster but not as precise.</param>
		/// <remarks>The IAsyncOperation result supports cancellation, so long running frame requests (exactSeek=true) can be interrupted.</remarks>
		IAsyncOperation<VideoFrame^>^ ExtractVideoFrameAsync(TimeSpan position, bool exactSeek) { return ExtractVideoFrameAsync(position, exactSeek, 0, nullptr); };

		/// <summary>Extracts a video frame at the specififed position.</summary>
		/// <param name="position">The position of the requested frame.</param>
		/// <remarks>The IAsyncOperation result supports cancellation, so long running frame requests (exactSeek=true) can be interrupted.</remarks>
		IAsyncOperation<VideoFrame^>^ ExtractVideoFrameAsync(TimeSpan position) { return ExtractVideoFrameAsync(position, false, 0, nullptr); };


		/// <summary>Extracts the next consecutive video frame in the file. Returns <c>null</c> at end of stream.</summary>
		IAsyncOperation<VideoFrame^>^ ExtractNextVideoFrameAsync() { return ExtractNextVideoFrameAsync(nullptr); };
	};
}



