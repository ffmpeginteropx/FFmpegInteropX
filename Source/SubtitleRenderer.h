#pragma once
#include "pch.h"
using namespace Windows::Media::Core;
using namespace Windows::Media::Playback;
using namespace Windows::Graphics::DirectX::Direct3D11;
using namespace Microsoft::Graphics::Canvas;

namespace FFmpegInteropX
{
	public ref class SubtitleRenderer sealed
	{

	public:
		SubtitleRenderingResult^ RenderSubtitleToSurface(MediaPlaybackItem^ item, IDirect3DSurface^ surface)
		{
			auto canvasDevice = CanvasDevice::GetSharedDevice();
			auto bitmap = CanvasBitmap::CreateFromDirect3D11Surface(canvasDevice, surface);
			auto renderTarget = CanvasRenderTarget::CreateFromDirect3D11Surface(canvasDevice, surface);
			auto ds = renderTarget->CreateDrawingSession();
			ds->Antialiasing = CanvasAntialiasing::Aliased;
			ds->Blend = CanvasBlend::Copy;
			ds->DrawImage(bitmap);

			for (unsigned int i = 0; i < item->TimedMetadataTracks->Size; i++)
			{
				auto track = item->TimedMetadataTracks->GetAt(i);
				if (item->TimedMetadataTracks->GetPresentationMode(i) != TimedMetadataTrackPresentationMode::PlatformPresented)
					continue;

				auto renderer = GetRenderer(track);

				auto activeCues = track->ActiveCues;
				for (auto ac : activeCues)
				{

				}
			}
		}

	private:
		SubtitleRendererInternal^ GetRenderer(TimedMetadataTrack^ track)
		{
			switch (track->TimedMetadataKind)
			{
				case TimedMetadataKind::ImageSubtitle: return ref new BitmapSubtitleRenderer();
				case TimedMetadataKind::Subtitle: return ref new AssSsaSubtitleRenderer();
			}

			return nullptr;
		}

	};

	public ref class SubtitleRenderingResult sealed
	{

	};

	ref class SubtitleRendererInternal abstract
	{
	public:
		virtual void RenderSubtitle(TimedMetadataTrack^ track, CanvasDrawingSession^ session) = 0;
	};

	ref class AssSsaSubtitleRenderer : public SubtitleRendererInternal
	{
	public:
		virtual void RenderSubtitle(TimedMetadataTrack^ track, CanvasDrawingSession^ session) override
		{

		}
	};

	ref class BitmapSubtitleRenderer : public SubtitleRendererInternal
	{
	public:
		virtual void RenderSubtitle(TimedMetadataTrack^ track, CanvasDrawingSession^ session) override
		{

		}
	};
}

