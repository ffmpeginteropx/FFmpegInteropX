#pragma once
#include "FFmpegInteropMSS.h"
#include "FFmpegInteropConfig.h"
#include "UncompressedVideoSampleProvider.h"
#include <ppl.h>

using namespace concurrency;

namespace FFmpegInterop {

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

		/// <summary>Gets video stream information.</summary>
		property VideoStreamInfo^ VideoStream
		{
			VideoStreamInfo^ get() { return interopMSS->VideoStream; }
		}

		/// <summary>Gets or sets the decode pixel width.</summary>
		property int DecodePixelWidth
		{
			int get() { return interopMSS->Configuration->DecodePixelWidth; }
			void set(int value) { interopMSS->Configuration->DecodePixelWidth = value; }
		}

		/// <summary>Gets or sets the decode pixel height.</summary>
		property int DecodePixelHeight
		{
			int get() { return interopMSS->Configuration->DecodePixelHeight; }
			void set(int value) { interopMSS->Configuration->DecodePixelHeight = value; }
		}

		/// <summary>Creates a new FrameGrabber from the specified stream.</summary>
		static IAsyncOperation<FrameGrabber^>^ CreateFromStreamAsync(IRandomAccessStream^ stream)
		{
			return create_async([stream]
			{
				FFmpegInteropConfig^ config = ref new FFmpegInteropConfig();
				config->IsFrameGrabber = true;

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
		/// <remarks>The IAsyncOperation result supports cancellation, so long running frame requests (exactSeek=true) can be interrupted.</remarks>
		IAsyncOperation<VideoFrame^>^ ExtractVideoFrameAsync(TimeSpan position, bool exactSeek, int maxFrameSkip)
		{
			return create_async([this, position, exactSeek, maxFrameSkip]
			{
				bool seekSucceeded = false;
				if (interopMSS->Duration.Duration > position.Duration)
				{
					seekSucceeded = SUCCEEDED(interopMSS->Seek(position));
				}

				int framesSkipped = 0;
				MediaStreamSample^ lastSample = nullptr;

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
							seekSucceeded = false;
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
					if (exactSeek && seekSucceeded && (position.Duration - sample->Timestamp.Duration > sample->Duration.Duration) &&
						(maxFrameSkip <= 0 || framesSkipped < maxFrameSkip))
					{
						framesSkipped++;
						continue;
					}

					//  make sure we have a clean sample (key frame, no half interlaced frame)
					if (!interopMSS->VideoSampleProvider->IsCleanSample && 
						(maxFrameSkip <= 0 || framesSkipped < maxFrameSkip))
					{
						framesSkipped++;
						continue;
					}

					auto sampleProvider = static_cast<UncompressedVideoSampleProvider^>(interopMSS->VideoSampleProvider);
					auto streamDescriptor = static_cast<VideoStreamDescriptor^>(interopMSS->VideoSampleProvider->StreamDescriptor);
					MediaRatio^ pixelAspectRatio = streamDescriptor->EncodingProperties->PixelAspectRatio;
					int width, height;
					if (interopMSS->Configuration->DecodePixelWidth > 0 || interopMSS->Configuration->DecodePixelHeight > 0)
					{
						width = sampleProvider->TargetWidth;
						height = sampleProvider->TargetHeight;
						pixelAspectRatio->Numerator = 1;
						pixelAspectRatio->Denominator = 1;
					}
					else
					{
						width = interopMSS->VideoStream->PixelWidth;
						height = interopMSS->VideoStream->PixelHeight;
					}

					auto result = ref new VideoFrame(sample->Buffer,
						width, height, pixelAspectRatio,
						sample->Timestamp);

					return result;

				}
			});
		}

		/// <summary>Extracts a video frame at the specififed position.</summary>
		/// <param name="position">The position of the requested frame.</param>
		/// <param name="exactSeek">If set to false, this will decode the closest previous key frame, which is faster but not as precise.</param>
		/// <remarks>The IAsyncOperation result supports cancellation, so long running frame requests (exactSeek=true) can be interrupted.</remarks>
		IAsyncOperation<VideoFrame^>^ ExtractVideoFrameAsync(TimeSpan position, bool exactSeek) { return ExtractVideoFrameAsync(position, exactSeek, 0); };
		
		/// <summary>Extracts a video frame at the specififed position.</summary>
		/// <param name="position">The position of the requested frame.</param>
		/// <remarks>The IAsyncOperation result supports cancellation, so long running frame requests (exactSeek=true) can be interrupted.</remarks>
		IAsyncOperation<VideoFrame^>^ ExtractVideoFrameAsync(TimeSpan position) { return ExtractVideoFrameAsync(position, false, 0); };

	};
}



