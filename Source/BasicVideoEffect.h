#pragma once
#include "BasicVideoEffect.g.h"
#include <winrt/Microsoft.Graphics.Canvas.h>
#include <winrt/Microsoft.Graphics.Canvas.Effects.h>

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropX::implementation
{
    using namespace Microsoft::Graphics::Canvas;
    using namespace Microsoft::Graphics::Canvas::Effects;
    using namespace Windows::Media::Effects;
    using namespace Windows::Media::MediaProperties;

    struct BasicVideoEffect : BasicVideoEffectT<BasicVideoEffect>
    {
        BasicVideoEffect() = default;

        void SetProperties(Windows::Foundation::Collections::IPropertySet const& configuration);
        bool IsReadOnly();
        MediaMemoryTypes SupportedMemoryTypes();
        bool TimeIndependent();
        Windows::Foundation::Collections::IVectorView<VideoEncodingProperties> SupportedEncodingProperties();
        void SetEncodingProperties(VideoEncodingProperties const& encodingProperties, Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice const& device);
        void ProcessFrame(ProcessVideoFrameContext const& context);
        void Close(MediaEffectClosedReason const& reason);
        void DiscardQueuedFrames();

    private:
        winrt::Windows::Foundation::Collections::IPropertySet inputConfiguration;
        CanvasDevice canvasDevice;
        winrt::FFmpegInteropX::VideoEffectConfiguration effectConfiguration;


        Effects::ICanvasEffect CreateColorEffect(ICanvasImage source, float c, float b, float s)
        {
            const auto lumR = 0.2125f;
            const auto lumG = 0.7154f;
            const auto lumB = 0.0721f;

            auto t = (1.0f - c) / 2.0f;
            auto sr = (1.0f - s) * lumR;
            auto sg = (1.0f - s) * lumG;
            auto sb = (1.0f - s) * lumB;

            Effects::Matrix5x4 matrix =
            {
                c * (sr + s),	c * (sr),		c * (sr),		0,
                c * (sg),		c * (sg + s),	c * (sg),		0,
                c * (sb),		c * (sb),		c * (sb + s),	0,
                0,				0,				0,				1,
                t + b,			t + b,			t + b,			0
            };

            auto colorMatrixEffect = Effects::ColorMatrixEffect();
            colorMatrixEffect.ColorMatrix(matrix);
            colorMatrixEffect.Source(source);

            return colorMatrixEffect;
        }

        Effects::ICanvasEffect CreateSharpnessEffect(ICanvasImage source, float sharpness, float sharpnessThreshold)
        {
            auto effect = Effects::SharpenEffect();
            effect.Amount(sharpness);
            effect.Threshold(sharpnessThreshold);
            effect.Source(source);
            return effect;
        }

        Effects::ICanvasEffect CreateTermperatureAndTintEffect(ICanvasImage source, float temperature, float tint)
        {
            auto effect = Effects::TemperatureAndTintEffect();
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
