#include "pch.h"
#include "VideoEffectProcessorExample.h"

void MediaPlayerCPP::VideoEffectProcessorExample::ProcessFrame(Windows::Graphics::DirectX::Direct3D11::IDirect3DSurface^ inputFrame, Windows::Graphics::DirectX::Direct3D11::IDirect3DSurface^ outputFrame)
{
    auto bla = inputFrame->Equals(outputFrame);
}
