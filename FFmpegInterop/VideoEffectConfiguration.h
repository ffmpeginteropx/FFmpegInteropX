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
	private:
		float _Brightness;
		float _Contrast;
		float _Saturation;
		float _Temperature;
		float _Tint;
		float _Sharpness;
		float _SharpnessThreshold;

	public:
		///<summary>Adjusts the brightness of the image. Default value is 0, range -1 to 1.</summary>
		property float Brightness
		{
			float get() { return _Brightness; }
			void set(float value) 
			{
				if (value < -1 || value > 1)
				{
					throw ref new InvalidArgumentException("Invalid parameter.");
				}
				_Brightness = value;
			}
		}

		///<summary>Adjusts the contrast of the image. Default value is 0, range -1 to 1.</summary>
		property float Contrast
		{
			float get() { return _Contrast; }
			void set(float value)
			{
				if (value < -1 || value > 1)
				{
					throw ref new InvalidArgumentException("Invalid parameter.");
				}
				_Contrast = value;
			}
		}

		///<summary>Adjusts the saturation of the image. Default value is 0, range -1 to 1.</summary>
		property float Saturation
		{
			float get() { return _Saturation; }
			void set(float value)
			{
				if (value < -1 || value > 1)
				{
					throw ref new InvalidArgumentException("Invalid parameter.");
				}
				_Saturation = value;
			}
		}

		///<summary>Adjusts the temperature of the image. Default value is 0, range -1 to 1.</summary>
		property float Temperature
		{
			float get() { return _Temperature; }
			void set(float value)
			{
				if (value < -1 || value > 1)
				{
					throw ref new InvalidArgumentException("Invalid parameter.");
				}
				_Temperature = value;
			}
		}

		///<summary>Adjusts the tint of the image. Default value is 0, range -1 to 1.</summary>
		property float Tint
		{
			float get() { return _Tint; }
			void set(float value)
			{
				if (value < -1 || value > 1)
				{
					throw ref new InvalidArgumentException("Invalid parameter.");
				}
				_Tint = value;
			}
		}

		///<summary>Adjusts the sharpness of the image. Default value is 0, range 0 to 10.</summary>
		property float Sharpness
		{
			float get() { return _Sharpness; }
			void set(float value)
			{
				if (value < 0 || value > 10)
				{
					throw ref new InvalidArgumentException("Invalid parameter.");
				}
				_Sharpness = value;
			}
		}

		///<summary>Adjusts which areas are sharpened. Default value is 0, range 0 to 10.</summary>
		property float SharpnessThreshold
		{
			float get() { return _SharpnessThreshold; }
			void set(float value)
			{
				if (value < 0 || value > 10)
				{
					throw ref new InvalidArgumentException("Invalid parameter.");
				}
				_SharpnessThreshold = value;
			}
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