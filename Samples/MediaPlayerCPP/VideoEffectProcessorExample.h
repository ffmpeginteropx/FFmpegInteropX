#pragma once
#include <windows.graphics.directx.direct3d11.interop.h>
#include <d3d11.h>
#include <mfidl.h>
#include <Microsoft.Graphics.Canvas.native.h>

using namespace Microsoft::Graphics::Canvas;
using namespace FFmpegInteropX;
using namespace Windows::Graphics::DirectX::Direct3D11;
using namespace Microsoft::Graphics::Canvas::Effects;

namespace MediaPlayerCPP
{
    public ref class VideoEffectProcessorExample sealed : IVideoFrameProcessor
    {
    public:
        VideoEffectProcessorExample(VideoEffectConfiguration^ _effectConfiguration)
        {
            EffectConfiguration = _effectConfiguration;
        }
        // Inherited via IVideoFrameProcessor
        virtual void ProcessFrame(Windows::Graphics::DirectX::Direct3D11::IDirect3DSurface^ inputFrame, Windows::Graphics::DirectX::Direct3D11::IDirect3DSurface^ outputFrame);

    private:
        HRESULT GetDXGISurface(IDirect3DSurface^ source, IDXGISurface** dxgiSurface)
        {
            return Windows::Graphics::DirectX::Direct3D11::GetDXGIInterface(source, dxgiSurface);
        }

        ICanvasEffect^ CreateColorEffect(ICanvasImage^ source, float c, float b, float s)
        {
            const auto lumR = 0.2125f;
            const auto lumG = 0.7154f;
            const auto lumB = 0.0721f;

            auto t = (1.0f - c) / 2.0f;
            auto sr = (1.0f - s) * lumR;
            auto sg = (1.0f - s) * lumG;
            auto sb = (1.0f - s) * lumB;

            Matrix5x4 matrix =
            {
                c * (sr + s),	c * (sr),		c * (sr),		0,
                c * (sg),		c * (sg + s),	c * (sg),		0,
                c * (sb),		c * (sb),		c * (sb + s),	0,
                0,				0,				0,				1,
                t + b,			t + b,			t + b,			0
            };

            auto colorMatrixEffect = ref new ColorMatrixEffect();
            colorMatrixEffect->ColorMatrix = matrix;
            colorMatrixEffect->Source = source;

            return colorMatrixEffect;
        }

        ICanvasEffect^ CreateSharpnessEffect(ICanvasImage^ source, float sharpness, float sharpnessThreshold)
        {
            auto effect = ref new SharpenEffect();
            effect->Amount = sharpness;
            effect->Threshold = sharpnessThreshold;
            effect->Source = source;
            return effect;
        }

        ICanvasEffect^ CreateTermperatureAndTintEffect(ICanvasImage^ source, float temperature, float tint)
        {
            auto effect = ref new TemperatureAndTintEffect();
            effect->Temperature = temperature;
            effect->Tint = tint;
            effect->Source = source;
            return effect;
        }
        VideoEffectConfiguration^ EffectConfiguration;
    };
}

