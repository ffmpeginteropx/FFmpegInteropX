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
using namespace Windows::UI::Text;

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

			for (auto ac : activeCues)
			{
				auto textCue = dynamic_cast<TimedTextCue^>(ac);
				auto cue = textCue;
				auto regionStyle = textCue->CueRegion;
				auto cueStyle = textCue->CueStyle;
				OutputDebugString((L"\n lines in cue " + cue->Lines->Size.ToString())->Data());
				//render each line, bottom up

				//X and Y location of each line
				float xLoc = regionStyle->Position.X;
				float yLoc = regionStyle->Position.Y;

				for (int i = textCue->Lines->Size - 1; i >= 0; i--)
				{
					for (int k = 0; k < 4; k++) //this is for testing multiple lines. To be REMOVED later
					{
						auto line = textCue->Lines->GetAt(i);
						// each subformat has its own style that needs to be rendered separatly
						for (auto subFormat : line->Subformats)
						{
							//this is used to compute the position inside the line for subformats
							float xSubFormatLineLoc = xLoc;
							auto lineText = line->Text + k.ToString();
							auto wLineText = StringUtils::PlatformStringToWString(lineText);
							auto subFormatStyle = subFormat->SubformatStyle;
							auto canvasFormat = ref new CanvasTextFormat();
							lineText = ref new String(wLineText.substr(subFormat->StartIndex, subFormat->Length).c_str());
							SetDisplayAlignment(canvasFormat, regionStyle, subFormatStyle);
							canvasFormat->FontSize = GetFontSize(subFormatStyle, width, height);
							canvasFormat->FontFamily = subFormatStyle->FontFamily;
							canvasFormat->FontStyle = GetFontStyle(subFormatStyle);
							canvasFormat->Direction = GetTextFlowDirection(subFormatStyle);
							canvasFormat->FontWeight = GetFontWeight(subFormatStyle);

							auto textLayout = ref new CanvasTextLayout(session->Device, lineText, canvasFormat, width, height);

							session->DrawTextLayout(textLayout, xSubFormatLineLoc, yLoc, Colors::White);
							yLoc -= textLayout->DrawBounds.Height;
							xSubFormatLineLoc += textLayout->DrawBounds.Width;
							OutputDebugString((L"\n DrawBounds.Width " + textLayout->DrawBounds.Width.ToString() + L"\n")->Data());
							OutputDebugString((L"\n DrawBounds.Height " + textLayout->DrawBounds.Height.ToString() + L"\n")->Data());
						}
					}
				}
			}
		}

		float GetFontSize(TimedTextStyle^ cueStyle, float width, float height)
		{
			auto fontSize = cueStyle->FontSize;
			if (fontSize.Unit == TimedTextUnit::Percentage)
			{
				//100 -> width
				//value -> x
				return (fontSize.Value * width) / 100;
			}
			return fontSize.Value;
		}

		FontStyle GetFontStyle(TimedTextStyle^ cueStyle)
		{
			auto cueFontStyle = cueStyle->FontStyle;
			switch (cueFontStyle)
			{
			case TimedTextFontStyle::Italic: return FontStyle::Italic;
			case TimedTextFontStyle::Normal: return FontStyle::Normal;
			case TimedTextFontStyle::Oblique: return FontStyle::Oblique;
			}
		}

		CanvasTextDirection GetTextFlowDirection(TimedTextStyle^ cueStyle)
		{
			auto cueFlowDirection = cueStyle->FlowDirection;
			switch (cueFlowDirection)
			{
			case TimedTextFlowDirection::LeftToRight: return CanvasTextDirection::LeftToRightThenTopToBottom;
			case TimedTextFlowDirection::RightToLeft: return CanvasTextDirection::RightToLeftThenTopToBottom;

			}
		}

		FontWeight GetFontWeight(TimedTextStyle^ cueStyle)
		{
			auto cueFontWeight = cueStyle->FontWeight;
			switch (cueFontWeight)
			{
			case TimedTextWeight::Bold: return FontWeights::Bold;
			case TimedTextWeight::Normal: return FontWeights::Normal;

			}
		}

		
		void SetDisplayAlignment(CanvasTextFormat^ textFormat, TimedTextRegion^ cueRegion, TimedTextStyle^ cueStyle)
		{
			//TimedTextDisplayAlignment -> VerticalAlignment -> TimedTextWritingMode 
			if (cueRegion->DisplayAlignment == TimedTextDisplayAlignment::Center)
			{
				textFormat->VerticalAlignment = CanvasVerticalAlignment::Center;
			}
			else if (cueRegion->DisplayAlignment == TimedTextDisplayAlignment::Before)
			{
				switch (cueRegion->WritingMode)
				{
				case TimedTextWritingMode::LeftRight:
					textFormat->VerticalAlignment = CanvasVerticalAlignment::Top;
					break;
				case TimedTextWritingMode::LeftRightTopBottom: 
					textFormat->VerticalAlignment = CanvasVerticalAlignment::Top;
					break;
				case TimedTextWritingMode::RightLeft: 
					textFormat->VerticalAlignment = CanvasVerticalAlignment::Top;
					break;
				case TimedTextWritingMode::RightLeftTopBottom:
					textFormat->VerticalAlignment = CanvasVerticalAlignment::Top;
					break;
				case TimedTextWritingMode::TopBottom: 
					textFormat->VerticalAlignment = CanvasVerticalAlignment::Top;
					break;
				case TimedTextWritingMode::TopBottomLeftRight:
					textFormat->VerticalAlignment = CanvasVerticalAlignment::Top;
					break;
				case TimedTextWritingMode::TopBottomRightLeft: 
					textFormat->VerticalAlignment = CanvasVerticalAlignment::Top;
					break;
				default:
					break;
				}
			}
			else if (cueRegion->DisplayAlignment == TimedTextDisplayAlignment::After)
			{
				switch (cueRegion->WritingMode)
				{
				case TimedTextWritingMode::LeftRight:
					textFormat->VerticalAlignment = CanvasVerticalAlignment::Bottom;
					break;
				case TimedTextWritingMode::LeftRightTopBottom:
					textFormat->VerticalAlignment = CanvasVerticalAlignment::Bottom;
					break;
				case TimedTextWritingMode::RightLeft:
					textFormat->VerticalAlignment = CanvasVerticalAlignment::Bottom;
					break;
				case TimedTextWritingMode::RightLeftTopBottom:
					textFormat->VerticalAlignment = CanvasVerticalAlignment::Bottom;
					break;
				case TimedTextWritingMode::TopBottom:
					textFormat->VerticalAlignment = CanvasVerticalAlignment::Bottom;
					break;
				case TimedTextWritingMode::TopBottomLeftRight:
					textFormat->VerticalAlignment = CanvasVerticalAlignment::Bottom;
					break;
				case TimedTextWritingMode::TopBottomRightLeft:
					textFormat->VerticalAlignment = CanvasVerticalAlignment::Bottom;
					break;
				default:
					break;
				}
			}

			//TimedTextLineAlignment    -> HorizontalAlignment -> TimedTextFlowDirection

			if (cueStyle->LineAlignment == TimedTextLineAlignment::Center)
			{
				textFormat->HorizontalAlignment = CanvasHorizontalAlignment::Center;
			}
			else if (cueStyle->LineAlignment == TimedTextLineAlignment::Start)
			{
				switch (cueStyle->FlowDirection)
				{
					case TimedTextFlowDirection::LeftToRight:
					textFormat->HorizontalAlignment = CanvasHorizontalAlignment::Left;
					break;
				case TimedTextFlowDirection::RightToLeft:
					textFormat->HorizontalAlignment = CanvasHorizontalAlignment::Right;
					break;
				}
			}
			else if (cueStyle->LineAlignment == TimedTextLineAlignment::End)
			{
				switch (cueStyle->FlowDirection)
				{
				case TimedTextFlowDirection::LeftToRight:
					textFormat->HorizontalAlignment = CanvasHorizontalAlignment::Right;
					break;
				case TimedTextFlowDirection::RightToLeft:
					textFormat->HorizontalAlignment = CanvasHorizontalAlignment::Left;
					break;
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

