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

		}

	};

	public ref class SubtitleRenderingResult sealed
	{

	};

	class SubtitleRendererInternal abstract
	{
	public:
		virtual void RenderSubtitle(TimedMetadataTrack^ track, CanvasDrawingSession^ session) = 0;
	};

	class AssSsaSubtitleRenderer : public SubtitleRendererInternal
	{
	public:
		virtual void RenderSubtitle(TimedMetadataTrack^ track, CanvasDrawingSession^ session)
		{

		}
	};

	class BitmapSubtitleRenderer : public SubtitleRendererInternal
	{
	public:
		virtual void RenderSubtitle(TimedMetadataTrack^ track, CanvasDrawingSession^ session)
		{

		}
	};
}

