#pragma once
#include "VideoEffectConfiguration.h"

using namespace Windows::Media::Playback;
using namespace Windows::Media::Core;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml::Controls;


namespace FFmpegInterop {

	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class MediaPlayerHelpers sealed
	{
	public:
		[Windows::Foundation::Metadata::DefaultOverloadAttribute]
		static VideoEffectConfiguration^ AddVideoEffect(MediaPlayer^ player, bool optional)
		{
			VideoEffectConfiguration^ config = ref new VideoEffectConfiguration();
			PropertySet^ set = ref new PropertySet();
			set->Insert("config", config);
			player->AddVideoEffect("FFmpegInterop.BasicVideoEffect", optional, set);
			return config;
		}

		[Windows::Foundation::Metadata::DefaultOverloadAttribute]
		static VideoEffectConfiguration^ AddVideoEffect(MediaPlayer^ player, VideoEffectConfiguration^ config, bool optional)
		{			
			PropertySet^ set = ref new PropertySet();
			set->Insert("config", config);
			player->AddVideoEffect("FFmpegInterop.BasicVideoEffect", optional, set);
			return config;
		}

		static VideoEffectConfiguration^ AddVideoEffect(MediaElement^ player, bool optional)
		{
			VideoEffectConfiguration^ config = ref new VideoEffectConfiguration();
			PropertySet^ set = ref new PropertySet();
			set->Insert("config", config);
			player->AddVideoEffect("FFmpegInterop.BasicVideoEffect", optional, set);
			return config;
		}

		static VideoEffectConfiguration^ AddVideoEffect(MediaElement^ player, VideoEffectConfiguration^ config, bool optional)
		{
			PropertySet^ set = ref new PropertySet();
			set->Insert("config", config);
			player->AddVideoEffect("FFmpegInterop.BasicVideoEffect", optional, set);
			return config;
		}

	private:
		MediaPlayerHelpers() { }
	};
}

