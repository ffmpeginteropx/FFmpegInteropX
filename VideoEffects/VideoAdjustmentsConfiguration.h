#pragma once
#ifdef UWP
#include "VideoAdjustmentsConfiguration.g.h"

#include <shcore.h>

#include <winrt/Windows.Foundation.Collections.h>

namespace winrt::FFmpegInteropX::VideoEffects::implementation
{
    struct VideoAdjustmentsConfiguration : VideoAdjustmentsConfigurationT<VideoAdjustmentsConfiguration>
    {
        VideoAdjustmentsConfiguration() = default;

        ///<summary>Adjusts the brightness of the image. Default value is 0, recommended range -1 to 1.</summary>
        float Brightness();
        void Brightness(float value);

        ///<summary>Adjusts the contrast of the image. Default value is 0, recommended range -1 to 1.</summary>
        float Contrast();
        void Contrast(float value);

        ///<summary>Adjusts the saturation of the image. Default value is 0, recommended range -1 to 1.</summary>
        float Saturation();
        void Saturation(float value);

        ///<summary>Adjusts the color temperature of the image. Default value is 0, allowed range -1 to 1.</summary>
        float Temperature();
        void Temperature(float value);

        ///<summary>Adjusts the tint of the image. Default value is 0, allowed range -1 to 1.</summary>
        float Tint();
        void Tint(float value);

        ///<summary>Adjusts the sharpness of the image. Default value is 0, allowed range 0 to 10.</summary>
        float Sharpness();
        void Sharpness(float value);

        ///<summary>Adjusts which areas are sharpened. Default value is 0, allowed range 0 to 10.</summary>
        float SharpnessThreshold();
        void SharpnessThreshold(float value);

        void AddVideoEffect(winrt::Windows::Media::Playback::MediaPlayer const& player);
        void AddVideoEffect(winrt::Windows::Media::Playback::MediaPlayer const& player, bool Optional);

    private:
        float _Brightness = 0;
        float _Contrast = 0;
        float _Saturation = 0;
        float _Temperature = 0;
        float _Tint = 0;
        float _Sharpness = 0;
        float _SharpnessThreshold = 0;
    };
}
namespace winrt::FFmpegInteropX::VideoEffects::factory_implementation
{
    struct VideoAdjustmentsConfiguration : VideoAdjustmentsConfigurationT<VideoAdjustmentsConfiguration, implementation::VideoAdjustmentsConfiguration>
    {
    };
}
#endif
