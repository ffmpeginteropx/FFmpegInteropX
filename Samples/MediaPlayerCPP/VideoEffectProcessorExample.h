#pragma once
using namespace FFmpegInteropX;

namespace MediaPlayerCPP
{
    public ref class VideoEffectProcessorExample sealed : IVideoFrameProcessor
    {
    public:
        VideoEffectProcessorExample() {}
        // Inherited via IVideoFrameProcessor
        virtual void ProcessFrame(Windows::Graphics::DirectX::Direct3D11::IDirect3DSurface^ inputFrame, Windows::Graphics::DirectX::Direct3D11::IDirect3DSurface^ outputFrame);
    };
}

