#pragma once
namespace FFmpegInterop {

	using namespace Windows::Storage::Streams;
	using namespace Windows::Graphics::DirectX::Direct3D11;

	public interface class IVideoFrameProcessor
	{
		IBuffer^ ProcessBuffer(IBuffer^ source);

		IDirect3DSurface^ ProcessDirectXSurface(IDirect3DSurface^ source);
	};
}

