#pragma once
#include <VideoEffectConfiguration.h>

namespace FFmpegInterop
{
	using namespace Platform::Collections;
	using namespace Windows::Media::Effects;
	using namespace Windows::Foundation::Collections;
	using namespace Windows::Media::MediaProperties;

	using namespace Microsoft::Graphics;
	using namespace Microsoft::Graphics::Canvas;
	using namespace Microsoft::Graphics::Canvas::Effects;

	public ref class BasicVideoEffect sealed : IBasicVideoEffect
	{
		IPropertySet^ inputConfiguration;
		CanvasDevice^ canvasDevice;
		VideoEffectConfiguration^ EffectConfiguration;

	public:
		virtual void SetProperties(IPropertySet ^configuration)
		{
			this->inputConfiguration = configuration;
			EffectConfiguration = safe_cast<VideoEffectConfiguration^>(configuration->Lookup("config"));
		}

		virtual property bool IsReadOnly
		{
			bool get()
			{
				return false;
			}
		}

		virtual property IVectorView<VideoEncodingProperties ^> ^ SupportedEncodingProperties
		{
			IVectorView<VideoEncodingProperties ^> ^ get()
			{
				auto encodingProperties = ref new VideoEncodingProperties();
				encodingProperties->Subtype = "ARGB32";
				auto vector = ref new Vector< VideoEncodingProperties^>();
				vector->Append(encodingProperties);
				return vector->GetView();
			}
		}

		virtual property MediaMemoryTypes SupportedMemoryTypes
		{
			MediaMemoryTypes get()
			{
				return MediaMemoryTypes::Gpu;
			}
		}

		virtual property bool TimeIndependent
		{
			bool get()
			{
				return true;
			}
		}

		virtual void SetEncodingProperties(Windows::Media::MediaProperties::VideoEncodingProperties ^encodingProperties, Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice ^device)
		{
			canvasDevice = CanvasDevice::CreateFromDirect3D11Device(device);
		}

		virtual void ProcessFrame(Windows::Media::Effects::ProcessVideoFrameContext ^context)
		{
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

				ICanvasImage^ source = CanvasBitmap::CreateFromDirect3D11Surface(canvasDevice, context->InputFrame->Direct3DSurface);

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

				auto renderTarget = CanvasRenderTarget::CreateFromDirect3D11Surface(canvasDevice, context->OutputFrame->Direct3DSurface);
				auto ds = renderTarget->CreateDrawingSession();
				ds->Antialiasing = CanvasAntialiasing::Aliased;
				ds->Blend = CanvasBlend::Copy;
				ds->DrawImage(source);
			}
			catch (...)
			{
			}
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

		virtual void Close(Windows::Media::Effects::MediaEffectClosedReason reason)
		{
			canvasDevice = nullptr;
		}

		virtual void DiscardQueuedFrames()
		{
		}
	};
}