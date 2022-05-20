#pragma once
#include "pch.h"
using namespace Windows::Media::Core;
using namespace Windows::Media::Playback;
using namespace Windows::Graphics::DirectX::Direct3D11;
using namespace Microsoft::Graphics::Canvas;
using namespace Microsoft::Graphics::Canvas::Text;
using namespace Windows::UI;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml::Media;
using namespace Microsoft::Graphics::Canvas::UI::Xaml;
using namespace Windows::Graphics::Imaging;

namespace FFmpegInteropX
{
	public ref class SubtitleRenderingResult sealed
	{
	private:
		CanvasImageSource^ imageSource;

	public:
		property ImageSource^ BitmapImageSource
		{
			public: ImageSource^ get()
			{
				return imageSource;
			}
		}

		SubtitleRenderingResult() 
		{
		}

		SubtitleRenderingResult(CanvasImageSource^ _imageSource)
		{
			imageSource = _imageSource;
		}
	};

	interface class SubtitleRendererInternal
	{
		void RenderSubtitle(TimedMetadataTrack^ track, CanvasDrawingSession^ session, CanvasBitmap^ inputBitmap);
	};

	ref class AssSsaSubtitleRenderer : public SubtitleRendererInternal
	{
	public:
		virtual void RenderSubtitle(TimedMetadataTrack^ track, CanvasDrawingSession^ session, CanvasBitmap^ inputBitmap)
		{
			auto activeCues = track->ActiveCues;
			for (auto ac : activeCues)
			{
				auto textCue = dynamic_cast<TimedTextCue^>(ac);

				auto cue = textCue;
				//render each line, bottom up
				for (int j = cue->Lines->Size - 1; j > 0; j--)
				{
					auto line = cue->Lines->GetAt(j);
					auto lineText = line->Text;

					auto canvasFormat = ref new CanvasTextFormat();
					canvasFormat->VerticalAlignment = CanvasVerticalAlignment::Bottom;
					canvasFormat->HorizontalAlignment = CanvasHorizontalAlignment::Center;

					auto textLayout = ref new CanvasTextLayout(session->Device, lineText, canvasFormat, (float)inputBitmap->Size.Width, (float)inputBitmap->Size.Height);

					/*auto theRectYouAreLookingFor = ref new Rect(xLoc + textLayout.DrawBounds.X,
						yLoc + textLayout.DrawBounds.Y,
						textLayout.DrawBounds.Width,
						textLayout.DrawBounds.Height);*/
					session->DrawTextLayout(textLayout, 0, 0, Colors::White);
				}
			}
		}
	};

	ref class BitmapSubtitleRenderer : public SubtitleRendererInternal
	{
	public:
		virtual void RenderSubtitle(TimedMetadataTrack^ track, CanvasDrawingSession^ session, CanvasBitmap^ inputBitmap)
		{
			auto activeCues = track->ActiveCues;
			for (auto ac : activeCues)
			{
				auto imageCue = dynamic_cast<ImageCue^>(ac);
				auto bitmap = CanvasBitmap::CreateFromSoftwareBitmap(session->Device, imageCue->SoftwareBitmap);

			}
		}
	};

	public ref class SubtitleRenderer sealed
	{
	private:
		SoftwareBitmap^ subtitleDest;
		CanvasImageSource^ subtitleImageSource;

	public:
		SubtitleRenderingResult^ RenderSubtitleToSurface(MediaPlaybackItem^ item, float width, float height)
		{
			auto canvasDevice = CanvasDevice::GetSharedDevice();

			if (subtitleDest == nullptr || (subtitleDest->PixelWidth != width) || (subtitleDest->PixelHeight != height))
			{
				subtitleDest = ref new SoftwareBitmap(BitmapPixelFormat::Bgra8, (int)width, (int)height, BitmapAlphaMode::Premultiplied);
			}

			if (subtitleImageSource == nullptr || (subtitleImageSource->Size.Width != width) || (subtitleImageSource->Size.Height != height))
			{
				subtitleImageSource = ref new  CanvasImageSource(canvasDevice, (float)width, (float)height, 96, CanvasAlphaMode::Premultiplied);
			}

			CanvasBitmap^ inputBitmap = CanvasBitmap::CreateFromSoftwareBitmap(canvasDevice, subtitleDest);
			CanvasDrawingSession^ ds = subtitleImageSource->CreateDrawingSession(Colors::Transparent);
			
			ds->Antialiasing = CanvasAntialiasing::Aliased;
			ds->Blend = CanvasBlend::Copy;

			for (unsigned int i = 0; i < item->TimedMetadataTracks->Size; i++)
			{
				if (item->TimedMetadataTracks->GetPresentationMode(i) != TimedMetadataTrackPresentationMode::PlatformPresented)
					continue;

				auto track = item->TimedMetadataTracks->GetAt(i);

				auto renderer = GetRenderer(track);

				renderer->RenderSubtitle(track, ds, nullptr);
			}
			ds->DrawImage(inputBitmap);

			return ref new SubtitleRenderingResult(subtitleImageSource);
		}

	private:
		SubtitleRendererInternal^ GetRenderer(TimedMetadataTrack^ track)
		{
			if (track->TimedMetadataKind == TimedMetadataKind::ImageSubtitle)
			{
				return ref new BitmapSubtitleRenderer();
			}
			if (track->TimedMetadataKind == TimedMetadataKind::Subtitle)
			{
				return ref new AssSsaSubtitleRenderer();
			}

			return nullptr;
		}
	};
}

