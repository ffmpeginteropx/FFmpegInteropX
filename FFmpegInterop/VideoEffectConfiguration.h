#pragma once
namespace FFmpegInterop
{
	using namespace Windows::Foundation::Collections;
	using namespace Windows::Media::Playback;
	using namespace Windows::UI::Xaml::Controls;

	/// <summary>Adjusts different aspects of the video image, GPU accelerated.
	/// Call AddVideoEffect to register with a MediaPlayer or MediaElement.</summary>
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class VideoEffectConfiguration sealed
	{
	public:
		///<summary>Adjusts the brightness of the image. Default value is 0.</summary>
		property float Brightness;

		///<summary>Adjusts the contrast of the image. Default value is 1.</summary>
		property float Contrast;

		///<summary>Adjusts the saturation of the image. Default value is 1.</summary>
		property float Saturation;

		///<summary>Adjusts the temperature of the image. Default value is 0, range -1 to 1.</summary>
		property float Temperature;

		///<summary>Adjusts the tint of the image. Default value is 0, range -1 to 1.</summary>
		property float Tint;

		///<summary>Adjusts the sharpness of the image. Default value is 0, range 0 to 10.</summary>
		property float Sharpness;

		///<summary>Adjusts which areas are sharpened. Default value is 0, range 0 to 10.</summary>
		property float SharpnessThreshold;

		VideoEffectConfiguration()
		{
			Sharpness = 0.0f;
			SharpnessThreshold = 0.0f;
			Contrast = 1.0f;
			Brightness = 0.0f;
			Saturation = 1.0f;
			Tint = 0.0f;
			Temperature = 0.0f;
		}

		[Windows::Foundation::Metadata::DefaultOverloadAttribute]
		void AddVideoEffect(MediaPlayer^ player)
		{
			PropertySet^ set = ref new PropertySet();
			set->Insert("config", this);
			player->AddVideoEffect("FFmpegInterop.BasicVideoEffect", true, set);
		}

		[Windows::Foundation::Metadata::DefaultOverloadAttribute]
		void AddVideoEffect(MediaPlayer^ player, bool optional)
		{
			PropertySet^ set = ref new PropertySet();
			set->Insert("config", this);
			player->AddVideoEffect("FFmpegInterop.BasicVideoEffect", optional, set);
		}

		void AddVideoEffect(MediaElement^ player)
		{
			VideoEffectConfiguration^ config = ref new VideoEffectConfiguration();
			PropertySet^ set = ref new PropertySet();
			set->Insert("config", config);
			player->AddVideoEffect("FFmpegInterop.BasicVideoEffect", true, set);
		}

		void AddVideoEffect(MediaElement^ player, bool optional)
		{
			VideoEffectConfiguration^ config = ref new VideoEffectConfiguration();
			PropertySet^ set = ref new PropertySet();
			set->Insert("config", config);
			player->AddVideoEffect("FFmpegInterop.BasicVideoEffect", optional, set);
		}
	};
}