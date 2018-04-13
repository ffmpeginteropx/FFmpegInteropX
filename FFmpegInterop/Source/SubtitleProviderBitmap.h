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
				if (m_pAvCodecCtx->extradata && m_pAvCodecCtx->extradata_size > 0)
				{
					auto str = std::string((char*)m_pAvCodecCtx->extradata, m_pAvCodecCtx->extradata_size);

					bool hasPalette;
					bool hasSize;

					auto sizePos = str.find("\nsize:");
					auto palettePos = str.find("\npalette:");
					/*auto orgPos = str.find("\norg:");
					auto scalePos = str.find("\nscale:");
					auto alphaPos = str.find("\nalpha:");*/

					if (sizePos >= 0 && palettePos >= 0)
					{
						auto count = sscanf_s((char*)m_pAvCodecCtx->extradata + 9,
							"%06x, %06x, %06x, %06x, %06x, %06x, %06x, %06x, "
							"%06x, %06x, %06x, %06x, %06x, %06x, %06x, %06x",
							&palette[0], &palette[1], &palette[2], &palette[3],
							&palette[4], &palette[5], &palette[6], &palette[7],
							&palette[8], &palette[9], &palette[10], &palette[11],
							&palette[12], &palette[13], &palette[14], &palette[15]);

						if (count > 0)
						{
							// convert from rgb to rgba
							for (int i = 0; i < count; i++)
							{
								palette[i] = palette[i] << 8 | 0x000000FF;
							}
							hasPalette = true;
						}

						count = sscanf_s((char*)m_pAvCodecCtx->extradata + 6, "%dx%d", &width, &height);
						if (count == 2)
						{
							hasSize = true;
						}
					}

					if (!hasPalette || !hasSize)
					{
						hr = E_FAIL;
					}
				}
			}

			return hr;
		}

		virtual IMediaCue^ CreateCue(AVPacket* packet) override
		{
			auto str = std::string((char*)packet->data, packet->size);
			if (packet->size != (width * height) / 2)
			{
				return nullptr;
			}

			using namespace Windows::Graphics::Imaging;

			auto bitmap = ref new SoftwareBitmap(BitmapPixelFormat::Bgra8, width, height);
			{
				auto buffer = bitmap->LockBuffer(BitmapBufferAccessMode::Write);

				// Query the IBufferByteAccess interface.  
				Microsoft::WRL::ComPtr<Windows::Foundation::IMemoryBufferByteAccess> bufferByteAccess;
				reinterpret_cast<IInspectable*>(buffer)->QueryInterface(IID_PPV_ARGS(&bufferByteAccess));

				// Retrieve the buffer data.  
				byte* pixels = nullptr;
				unsigned int capacity;
				bufferByteAccess->GetBuffer(&pixels, &capacity);

				auto plane = buffer->GetPlaneDescription(0);
				for (int i = 0; i < plane.Height; i++)
				{
					for (int j = 0; j < plane.Width; j++)
					{
						auto dataPosition = (i * width + j) / 2;
						byte data = packet->data[dataPosition];
						byte color;
						if (i % 2 == 0)
						{
							color = data & 0xF0 >> 4;
						}
						else
						{
							color = data & 0x0F;
						}
						auto pointer = pixels + plane.StartIndex + plane.Stride * i + 4 * j;
						((uint32*)pointer)[0] = palette[color];
					}
				}
			}
			
			ImageCue^ cue = ref new ImageCue();
			cue->SoftwareBitmap = bitmap;
			/*cue->Position = position;
			cue->Extent = extent;*/

			return cue;
		}

	private:
		int width;
		int height;
		uint32 palette[16];
		TimedTextSize extent;
		TimedTextPoint position;
	};
}