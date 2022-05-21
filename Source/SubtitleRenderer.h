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
using namespace Windows::Foundation;

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
		void RenderSubtitle(TimedMetadataTrack^ track, CanvasDrawingSession^ session, CanvasBitmap^ inputBitmap, float width, float height);
	};

	ref class AssSsaSubtitleRenderer : public SubtitleRendererInternal
	{
	public:
		virtual void RenderSubtitle(TimedMetadataTrack^ track, CanvasDrawingSession^ session, CanvasBitmap^ inputBitmap, float width, float height)
		{
			auto activeCues = track->ActiveCues;
			float xLoc = 0.0f;
			float yLoc = 0.0f;
			for (auto ac : activeCues)
			{
				auto textCue = dynamic_cast<TimedTextCue^>(ac);
				auto cue = textCue;
				OutputDebugString((L"\n lines in cue " + cue->Lines->Size.ToString())->Data());
				//render each line, bottom up
				for (int i = textCue->Lines->Size - 1; i >= 0; i--)
				{

					for (int k = 0; k < 4; k++) {
						auto line = textCue->Lines->GetAt(i);
						auto lineText = line->Text + k.ToString();

						auto canvasFormat = ref new CanvasTextFormat();
						canvasFormat->VerticalAlignment = CanvasVerticalAlignment::Bottom;
						canvasFormat->HorizontalAlignment = CanvasHorizontalAlignment::Center;
						canvasFormat->FontSize = 44;

						auto textLayout = ref new CanvasTextLayout(session->Device, lineText, canvasFormat, width, height);

						session->DrawTextLayout(textLayout, xLoc, yLoc, Colors::White);
						yLoc -= textLayout->DrawBounds.Height;
						OutputDebugString((L"\n DrawBounds.Width " + textLayout->DrawBounds.Width.ToString() + L"\n")->Data());
						OutputDebugString((L"\n DrawBounds.Height " + textLayout->DrawBounds.Height.ToString() + L"\n")->Data());
					}
				}
			}
		}
	};

	ref class BitmapSubtitleRenderer : public SubtitleRendererInternal
	{
	public:
		virtual void RenderSubtitle(TimedMetadataTrack^ track, CanvasDrawingSession^ session, CanvasBitmap^ inputBitmap, float width, float height)
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

				renderer->RenderSubtitle(track, ds, nullptr, width, height);
			}
			//ds->DrawImage(inputBitmap);
			ds->Flush();
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

