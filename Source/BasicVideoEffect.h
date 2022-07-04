#pragma once
#include "BasicVideoEffect.g.h"
#include <winrt/Microsoft.Graphics.Canvas.h>
#include <winrt/Microsoft.Graphics.Canvas.Effects.h>

using namespace winrt::Microsoft::Graphics::Canvas::Effects;
using namespace winrt::Microsoft::Graphics::Canvas;

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropX::implementation
{
    struct BasicVideoEffect : BasicVideoEffectT<BasicVideoEffect>
    {
        BasicVideoEffect() = default;

        void SetProperties(Windows::Foundation::Collections::IPropertySet const& configuration);
        bool IsReadOnly();
        Windows::Media::Effects::MediaMemoryTypes SupportedMemoryTypes();
        bool TimeIndependent();
        Windows::Foundation::Collections::IVectorView<Windows::Media::MediaProperties::VideoEncodingProperties> SupportedEncodingProperties();
        void SetEncodingProperties(Windows::Media::MediaProperties::VideoEncodingProperties const& encodingProperties, Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice const& device);
        void ProcessFrame(Windows::Media::Effects::ProcessVideoFrameContext const& context);
        void Close(Windows::Media::Effects::MediaEffectClosedReason const& reason);
        void DiscardQueuedFrames();

    private:
        winrt::Windows::Foundation::Collections::IPropertySet inputConfiguration;
        winrt::Microsoft::Graphics::Canvas::CanvasDevice canvasDevice;
        winrt::FFmpegInteropX::VideoEffectConfiguration effectConfiguration;


        winrt::Microsoft::Graphics::Canvas::Effects::ICanvasEffect CreateColorEffect(winrt::Microsoft::Graphics::Canvas::ICanvasImage source, float c, float b, float s)
        {
            const auto lumR = 0.2125f;
            const auto lumG = 0.7154f;
            const auto lumB = 0.0721f;

            auto t = (1.0f - c) / 2.0f;
            auto sr = (1.0f - s) * lumR;
            auto sg = (1.0f - s) * lumG;
            auto sb = (1.0f - s) * lumB;

            winrt::Microsoft::Graphics::Canvas::Effects::Matrix5x4 matrix =
            {
                c * (sr + s),	c * (sr),		c * (sr),		0,
                c * (sg),		c * (sg + s),	c * (sg),		0,
                c * (sb),		c * (sb),		c * (sb + s),	0,
                0,				0,				0,				1,
                t + b,			t + b,			t + b,			0
            };

            auto colorMatrixEffect = winrt::Microsoft::Graphics::Canvas::Effects::ColorMatrixEffect();
            colorMatrixEffect.ColorMatrix(matrix);
            colorMatrixEffect.Source(source);

            return colorMatrixEffect;
        }

        winrt::Microsoft::Graphics::Canvas::Effects::ICanvasEffect CreateSharpnessEffect(winrt::Microsoft::Graphics::Canvas::ICanvasImage source, float sharpness, float sharpnessThreshold)
        {
            auto effect = winrt::Microsoft::Graphics::Canvas::Effects::SharpenEffect();
            effect.Amount(sharpness);
            effect.Threshold(sharpnessThreshold);
            effect.Source(source);
            return effect;
        }

        winrt::Microsoft::Graphics::Canvas::Effects::ICanvasEffect CreateTermperatureAndTintEffect(winrt::Microsoft::Graphics::Canvas::ICanvasImage source, float temperature, float tint)
        {
            auto effect = winrt::Microsoft::Graphics::Canvas::Effects::TemperatureAndTintEffect();
            effect.Temperature(temperature);
            effect.Tint(tint);
            effect.Source(source);
            return effect;
        }

    };
}

namespace winrt::FFmpegInteropX::factory_implementation
{
    struct BasicVideoEffect : BasicVideoEffectT<BasicVideoEffect, implementation::BasicVideoEffect>
    {
    };
}
