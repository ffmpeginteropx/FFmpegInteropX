#pragma once
using namespace Platform::Collections;
using namespace Windows::Media::Effects;
using namespace Windows::Foundation::Collections;
using namespace Windows::Media::MediaProperties;


namespace FFmpegInterop
{
	public ref class BasicVideoEffect sealed : IBasicVideoEffect
	{
		IPropertySet^ configuration;
		// Inherited via IBasicVideoEffect
	public:
		virtual void SetProperties(IPropertySet ^configuration)
		{
			this->configuration = configuration;
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
				return MediaMemoryTypes::GpuAndCpu;
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

		}

		virtual void ProcessFrame(Windows::Media::Effects::ProcessVideoFrameContext ^context)
		{

		}

		virtual void Close(Windows::Media::Effects::MediaEffectClosedReason reason)
		{

		}

		virtual void DiscardQueuedFrames()
		{

		}
	};
}