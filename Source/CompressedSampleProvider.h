#pragma once

#include "MediaSampleProvider.h"

namespace FFmpegInteropX
{
    ref class CompressedSampleProvider : MediaSampleProvider
    {
    public:
        virtual ~CompressedSampleProvider();

    internal:
        CompressedSampleProvider(FFmpegReader^ reader, AVFormatContext* avFormatCtx, AVCodecContext* avCodecCtx, MediaSourceConfig^ config, int streamIndex, HardwareDecoderStatus hardwareDecoderStatus);
        CompressedSampleProvider(FFmpegReader^ reader, AVFormatContext* avFormatCtx, AVCodecContext* avCodecCtx, MediaSourceConfig^ config, int streamIndex, VideoEncodingProperties^ encodingProperties, HardwareDecoderStatus hardwareDecoderStatus);
        CompressedSampleProvider(FFmpegReader^ reader, AVFormatContext* avFormatCtx, AVCodecContext* avCodecCtx, MediaSourceConfig^ config, int streamIndex, AudioEncodingProperties^ encodingProperties, HardwareDecoderStatus hardwareDecoderStatus);
        virtual HRESULT CreateNextSampleBuffer(IBuffer^* pBuffer, int64_t& samplePts, int64_t& sampleDuration, IDirect3DSurface^* surface) override;
        virtual HRESULT CreateBufferFromPacket(AVPacket* avPacket, IBuffer^* pBuffer);
        virtual IMediaStreamDescriptor^ CreateStreamDescriptor() override;

    private:
        VideoEncodingProperties^ videoEncodingProperties;
        AudioEncodingProperties^ audioEncodingProperties;
    };
}