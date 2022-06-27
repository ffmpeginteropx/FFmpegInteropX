#include "pch.h"
#include "BasicVideoEffect.h"
#include "BasicVideoEffect.g.cpp"

using namespace winrt::Windows::Foundation::Collections;
using namespace winrt::Windows::Media::Effects;
using namespace winrt::Windows::Foundation::Collections;
using namespace winrt::Windows::Media::MediaProperties;

using namespace winrt::Microsoft::Graphics;
using namespace winrt::Microsoft::Graphics::Canvas;
using namespace winrt::Microsoft::Graphics::Canvas::Effects;

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropXWinUI::implementation
{
    void BasicVideoEffect::SetProperties(Windows::Foundation::Collections::IPropertySet const& configuration)
    {
        this->inputConfiguration = configuration;
        EffectConfiguration = winrt::unbox_value< winrt::FFmpegInteropXWinUI::VideoEffectConfiguration>(configuration.Lookup(L"config"));
    }

    bool BasicVideoEffect::IsReadOnly()
    {
        return false;
    }

    Windows::Media::Effects::MediaMemoryTypes BasicVideoEffect::SupportedMemoryTypes()
    {
        return winrt::Windows::Media::Effects::MediaMemoryTypes::Gpu;
    }

    bool BasicVideoEffect::TimeIndependent()
    {
        return true;
    }

    Windows::Foundation::Collections::IVectorView<Windows::Media::MediaProperties::VideoEncodingProperties> BasicVideoEffect::SupportedEncodingProperties()
    {
        auto encodingProperties = VideoEncodingProperties();
        encodingProperties.Subtype(L"ARGB32");
        auto vector = winrt::single_threaded_vector<VideoEncodingProperties>();
        vector.Append(encodingProperties);
        return vector.GetView();
    }

    void BasicVideoEffect::SetEncodingProperties(Windows::Media::MediaProperties::VideoEncodingProperties const& encodingProperties, Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice const& device)
    {
        canvasDevice = CanvasDevice::CreateFromDirect3D11Device(device);
    }

    void BasicVideoEffect::ProcessFrame(Windows::Media::Effects::ProcessVideoFrameContext const& context)
    {
        try
        {
            auto c = EffectConfiguration.Contrast();
            auto b = EffectConfiguration.Brightness();
            auto s = EffectConfiguration.Saturation();
            auto temp = EffectConfiguration.Temperature();
            auto tint = EffectConfiguration.Tint();
            auto sharpness = EffectConfiguration.Sharpness();
            auto sharpnessThreshold = EffectConfiguration.SharpnessThreshold();

            bool hasSharpness = sharpness > 0.0f;
            bool hasColor = c != 0.0f || b != 0.0f || s != 0.0f;
            bool hasTemperatureAndTint = tint != 0.0f || temp != 0.0f;

            ICanvasImage source = CanvasBitmap::CreateFromDirect3D11Surface(canvasDevice, context.InputFrame().Direct3DSurface());

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

            auto renderTarget = CanvasRenderTarget::CreateFromDirect3D11Surface(canvasDevice, context.OutputFrame().Direct3DSurface());
            auto ds = renderTarget.CreateDrawingSession();
            ds.Antialiasing(CanvasAntialiasing::Aliased);
            ds.Blend(CanvasBlend::Copy);
            ds.DrawImage(source);
        }
        catch (...)
        {
        }
    }

    void BasicVideoEffect::Close(Windows::Media::Effects::MediaEffectClosedReason const& reason)
    {
        canvasDevice = nullptr;
    }

    void BasicVideoEffect::DiscardQueuedFrames()
    {
    }
}
