#pragma once

#include "SubtitleProvider.h"
#include <MemoryBuffer.h>

namespace FFmpegInterop
{
	ref class SubtitleProviderBitmap : SubtitleProvider
	{
	internal:
		SubtitleProviderBitmap(FFmpegReader^ reader,
			AVFormatContext* avFormatCtx,
			AVCodecContext* avCodecCtx,
			FFmpegInteropConfig^ config,
			int index)
			: SubtitleProvider(reader, avFormatCtx, avCodecCtx, config, index, TimedMetadataKind::ImageSubtitle)
		{
		}

		virtual HRESULT Initialize() override
		{
			auto hr = SubtitleProvider::Initialize();
			if (SUCCEEDED(hr))
			{
				position.X = 0;
				position.Y = 0;
				position.Unit = TimedTextUnit::Percentage;

				extent.Width = 100;
				extent.Height = 100;
				extent.Unit = TimedTextUnit::Percentage;
			}

			return hr;
		}

		virtual void NotifyVideoFrameSize(int width, int height, double aspectRatio) override
		{
			videoWidth = width;
			videoHeight = height;
			if (isnormal(aspectRatio) && aspectRatio > 0)
			{
				videoAspectRatio = aspectRatio;
			}
			else
			{
				videoAspectRatio = (double)width / height;
			}
		}

		virtual IMediaCue^ CreateCue(AVPacket* packet, TimeSpan* position, TimeSpan *duration) override
		{
			// only decode image subtitles if the stream is selected
			if (!IsEnabled)
			{
				return nullptr;
			}

			AVSubtitle subtitle;
			int gotSubtitle = 0;
			auto result = avcodec_decode_subtitle2(m_pAvCodecCtx, &subtitle, &gotSubtitle, packet);
			if (result > 0 && gotSubtitle)
			{
				if (infiniteDurationCue)
				{
					TimeSpan dur;
					dur.Duration = position->Duration - infiniteDurationCue->StartTime.Duration;
					if (dur.Duration < 0)
					{
						dur.Duration = 30000000; //TODO improve with default duration from config, once other PR is merged
					}
					infiniteDurationCue->Duration = dur;
					infiniteDurationCue = nullptr;
				}

				int width, height, offsetY;
				if (subtitle.num_rects > 0 && CheckSize(subtitle, width, height, offsetY))
				{
					if (subtitle.start_display_time > 0)
					{
						position->Duration += 10000 * subtitle.start_display_time;
					}
					duration->Duration = 10000 * subtitle.end_display_time;

					using namespace Windows::Graphics::Imaging;

					auto bitmap = ref new SoftwareBitmap(BitmapPixelFormat::Bgra8, width, height, BitmapAlphaMode::Straight);
					{
						auto buffer = bitmap->LockBuffer(BitmapBufferAccessMode::Write);
						auto reference = buffer->CreateReference();

						// Query the IBufferByteAccess interface.  
						Microsoft::WRL::ComPtr<Windows::Foundation::IMemoryBufferByteAccess> bufferByteAccess;
						reinterpret_cast<IInspectable*>(reference)->QueryInterface(IID_PPV_ARGS(&bufferByteAccess));

						// Retrieve the buffer data.  
						byte* pixels = nullptr;
						unsigned int capacity;
						bufferByteAccess->GetBuffer(&pixels, &capacity);

						auto plane = buffer->GetPlaneDescription(0);

						for (unsigned int i = 0; i < subtitle.num_rects; i++)
						{
							auto rect = subtitle.rects[i];

							for (int y = 0; y < rect->h; y++)
							{
								for (int x = 0; x < rect->w; x++)
								{
									auto inPointer = rect->data[0] + y * rect->linesize[0] + x;
									auto color = inPointer[0];
									if (color < rect->nb_colors)
									{
										auto rgba = ((uint32*)rect->data[1])[color];
										auto outPointer = pixels + plane.StartIndex + plane.Stride * ((y + rect->y) - offsetY) + 4 * (x + rect->x);
										((uint32*)outPointer)[0] = rgba;
									}
									else
									{
										OutputDebugString(L"Error: Illegal subtitle color.");
									}
								}
							}
						}
					}

					ImageCue^ cue = ref new ImageCue();
					cue->SoftwareBitmap = SoftwareBitmap::Convert(bitmap, BitmapPixelFormat::Bgra8, BitmapAlphaMode::Premultiplied);
					cue->Position = this->position;
					cue->Extent = extent;

					if (subtitle.end_display_time = 0xFFFFFFFF)
					{
						infiniteDurationCue = cue;
					}

					avsubtitle_free(&subtitle);

					return cue;
				}
				else if (subtitle.num_rects > 0)
				{
					OutputDebugString(L"Error: Invalid subtitle size received.");
				}

				avsubtitle_free(&subtitle);
			}
			else if (result <= 0)
			{
				OutputDebugString(L"Failed to decode subtitle.");
			}

			return nullptr;
		}

	public:

		void Flush() override
		{
			SubtitleProvider::Flush();

			if (infiniteDurationCue)
			{
				//TODO improve
				infiniteDurationCue->Duration = { 30000000 };
				infiniteDurationCue = nullptr;
			}
		}

	private:

		bool CheckSize(AVSubtitle& subtitle, int& width, int& height, int& offsetY)
		{
			if (!hasSize)
			{
				subtitleWidth = m_pAvCodecCtx->width;
				subtitleHeight = m_pAvCodecCtx->height;

				if (subtitleWidth <= 0 || subtitleHeight <= 0)
				{
					return false;
				}

				if (subtitleWidth != videoWidth || subtitleHeight != videoHeight || (videoAspectRatio > 0 && videoAspectRatio != 1))
				{
					auto height = (int)(subtitleWidth / videoAspectRatio);
					if (height < subtitleHeight)
					{
						optimalHeight = height;
					}
				}

				hasSize = true;
			}

			width = subtitleWidth;
			height = subtitleHeight;
			offsetY = 0;

			if (optimalHeight)
			{
				int offset = (subtitleHeight - optimalHeight) / 2;
				for (unsigned int i = 0; i < subtitle.num_rects; i++)
				{
					auto rect = subtitle.rects[i];

					if (rect->y < offset || rect->y - offset + rect->h >= optimalHeight)
					{
						// does not fit into optimal size. disable from now on.
						optimalHeight = 0;
						break;
					}
				}

				if (optimalHeight != 0)
				{
					height = optimalHeight;
					offsetY = offset;
				}
			}

			for (unsigned int i = 0; i < subtitle.num_rects; i++)
			{
				auto rect = subtitle.rects[i];
				if (rect->y < offsetY || rect->y - offsetY + rect->h >= height ||
					rect->x < 0 || rect->x >= width)
				{
					// invalid size
					return false;
				}
			}

			return true;
		}

	private:
		int videoWidth;
		int videoHeight;
		double videoAspectRatio;
		bool hasSize;
		int subtitleWidth;
		int subtitleHeight;
		int optimalHeight;
		TimedTextSize extent;
		TimedTextPoint position;
		ImageCue^ infiniteDurationCue;
	};
}