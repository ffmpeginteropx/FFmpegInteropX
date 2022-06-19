#pragma once
#include <pch.h>
#include "MediaSampleProvider.h"

namespace FFmpegInteropX
{
	class CompressedSampleProvider : public MediaSampleProvider
	{
	public:
		virtual ~CompressedSampleProvider();

		CompressedSampleProvider(std::shared_ptr<FFmpegReader> reader, AVFormatContext* avFormatCtx, AVCodecContext* avCodecCtx, winrt::FFmpegInteropXWinUI::MediaSourceConfig const& config, int streamIndex, HardwareDecoderStatus hardwareDecoderStatus);
		CompressedSampleProvider(std::shared_ptr<FFmpegReader> reader, AVFormatContext* avFormatCtx, AVCodecContext* avCodecCtx, winrt::FFmpegInteropXWinUI::MediaSourceConfig const& config, int streamIndex, winrt::Windows::Media::MediaProperties::VideoEncodingProperties const& encodingProperties, HardwareDecoderStatus hardwareDecoderStatus);
		CompressedSampleProvider(std::shared_ptr<FFmpegReader> reader, AVFormatContext* avFormatCtx, AVCodecContext* avCodecCtx, winrt::FFmpegInteropXWinUI::MediaSourceConfig const& config, int streamIndex, winrt::Windows::Media::MediaProperties::AudioEncodingProperties const& encodingProperties, HardwareDecoderStatus hardwareDecoderStatus);
		virtual HRESULT CreateNextSampleBuffer(winrt::Windows::Storage::Streams::IBuffer* pBuffer, int64_t& samplePts, int64_t& sampleDuration, winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DSurface* surface) override;
		virtual HRESULT CreateBufferFromPacket(AVPacket* avPacket, winrt::Windows::Storage::Streams::IBuffer* pBuffer);
		virtual winrt::Windows::Media::Core::IMediaStreamDescriptor CreateStreamDescriptor() override;

	private:
		winrt::Windows::Media::MediaProperties::VideoEncodingProperties videoEncodingProperties = { nullptr };
		winrt::Windows::Media::MediaProperties::AudioEncodingProperties audioEncodingProperties = { nullptr };
	};
}