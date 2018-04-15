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
				position.Unit = TimedTextUnit::Pixels;

				extent.Width = 720;
				extent.Height = 576;
				extent.Unit = TimedTextUnit::Pixels;
			}

			return hr;
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
				if (subtitle.start_display_time > 0)
				{
					position->Duration += 10000 * subtitle.start_display_time;
				}
				duration->Duration = 10000 * subtitle.end_display_time;

				using namespace Windows::Graphics::Imaging;

				auto bitmap = ref new SoftwareBitmap(BitmapPixelFormat::Bgra8, 720, 576, BitmapAlphaMode::Premultiplied);
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

					for (int i = 0; i < subtitle.num_rects; i++)
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
									auto rgba = color == 0 ? 0 : ((uint32*)rect->data[1])[color] | 0x000000FF;
								
									auto outPointer = pixels + plane.StartIndex + plane.Stride * (y + rect->y) + 4 * (x + rect->x);
									((uint32*)outPointer)[0] = rgba;
								}
								else
								{
									//illegal color value
								}
							}
						}
					}

					bufferByteAccess = nullptr;
					delete reference;
					delete buffer;
				}

				ImageCue^ cue = ref new ImageCue();
				cue->SoftwareBitmap = bitmap;
				cue->Position = this->position;
				cue->Extent = extent;

				avsubtitle_free(&subtitle);

				return cue;
			}
			else
			{
				//log
			}

			return nullptr;
		}

	private:
		int width;
		int height;
		uint32 palette[16];
		TimedTextSize extent;
		TimedTextPoint position;
	};
}