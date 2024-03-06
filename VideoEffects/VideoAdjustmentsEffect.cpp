#ifdef UWP
#include "VideoAdjustmentsEffect.h"
#include "VideoAdjustmentsEffect.g.cpp"

using namespace winrt::Windows::Foundation::Collections;
using namespace winrt::Windows::Media::Effects;
using namespace winrt::Windows::Foundation::Collections;
using namespace winrt::Windows::Media::MediaProperties;

using namespace winrt::Microsoft::Graphics;
using namespace winrt::Microsoft::Graphics::Canvas;
using namespace winrt::Microsoft::Graphics::Canvas::Effects;

namespace winrt::FFmpegInteropX::VideoEffects::implementation
{
    using namespace Effects;
    using namespace Microsoft::Graphics::Canvas;
    using namespace Windows::Foundation::Collections;
    using namespace Windows::Media::MediaProperties;

    void VideoAdjustmentsEffect::SetProperties(IPropertySet const& configuration)
    {
        inputConfiguration = configuration;
        effectConfiguration = winrt::unbox_value< winrt::FFmpegInteropX::VideoEffects::VideoAdjustmentsConfiguration>(configuration.Lookup(L"config"));
    }

    bool VideoAdjustmentsEffect::IsReadOnly()
    {
        return false;
    }

    MediaMemoryTypes VideoAdjustmentsEffect::SupportedMemoryTypes()
    {
        return MediaMemoryTypes::Gpu;
    }

    bool VideoAdjustmentsEffect::TimeIndependent()
    {
        return true;
    }

    IVectorView<VideoEncodingProperties> VideoAdjustmentsEffect::SupportedEncodingProperties()
    {
        auto encodingProperties = VideoEncodingProperties();
        encodingProperties.Subtype(L"ARGB32");
        auto vector = winrt::single_threaded_vector<VideoEncodingProperties>();
        vector.Append(encodingProperties);
        return vector.GetView();
    }

    void VideoAdjustmentsEffect::SetEncodingProperties(VideoEncodingProperties const& encodingProperties, Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice const& device)
    {
        UNREFERENCED_PARAMETER(encodingProperties);
        canvasDevice = CanvasDevice::CreateFromDirect3D11Device(device);
    }

    void VideoAdjustmentsEffect::ProcessFrame(ProcessVideoFrameContext const& context)
    {
        try
        {
            auto c = effectConfiguration.Contrast();
            auto b = effectConfiguration.Brightness();
            auto s = effectConfiguration.Saturation();
            auto temp = effectConfiguration.Temperature();
            auto tint = effectConfiguration.Tint();
            auto sharpness = effectConfiguration.Sharpness();
            auto sharpnessThreshold = effectConfiguration.SharpnessThreshold();

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

    void VideoAdjustmentsEffect::Close(MediaEffectClosedReason const& reason)
    {
        UNREFERENCED_PARAMETER(reason);
        canvasDevice = nullptr;
    }

    void VideoAdjustmentsEffect::DiscardQueuedFrames()
    {
    }
}

#endif // UWP
