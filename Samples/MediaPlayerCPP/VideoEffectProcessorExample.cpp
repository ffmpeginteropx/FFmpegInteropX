#include "pch.h"
#include "VideoEffectProcessorExample.h"
using namespace Microsoft::WRL;

void MediaPlayerCPP::VideoEffectProcessorExample::ProcessFrame(Windows::Graphics::DirectX::Direct3D11::IDirect3DSurface^ inputFrame, Windows::Graphics::DirectX::Direct3D11::IDirect3DSurface^ outputFrame)
{
    CanvasDevice^ canvasDevice = CanvasDevice::GetSharedDevice();

    try
    {
        auto c = EffectConfiguration->Contrast;
        auto b = EffectConfiguration->Brightness;
        auto s = EffectConfiguration->Saturation;
        auto temp = EffectConfiguration->Temperature;
        auto tint = EffectConfiguration->Tint;
        auto sharpness = EffectConfiguration->Sharpness;
        auto sharpnessThreshold = EffectConfiguration->SharpnessThreshold;

        bool hasSharpness = sharpness > 0.0f;
        bool hasColor = c != 0.0f || b != 0.0f || s != 0.0f;
        bool hasTemperatureAndTint = tint != 0.0f || temp != 0.0f;

        ICanvasImage^ source = CanvasBitmap::CreateFromDirect3D11Surface(canvasDevice, inputFrame);

        if (hasColor)
        {
            source = CreateColorEffect(source, c + 1.0f, b, s + 1.0f);
        }

        if (hasTemperatureAndTint)
        {
            source = CreateTermperatureAndTintEffect(source, temp, tint);
        }

        if (hasSharpness)
        {
            source = CreateSharpnessEffect(source, sharpness, sharpnessThreshold);
        }

        auto renderTarget = CanvasRenderTarget::CreateFromDirect3D11Surface(canvasDevice, outputFrame);
        auto ds = renderTarget->CreateDrawingSession();
        ds->Antialiasing = CanvasAntialiasing::Aliased;
        ds->Blend = CanvasBlend::Copy;
        ds->DrawImage(source);
    }
    catch (...)
    {
    }
}
