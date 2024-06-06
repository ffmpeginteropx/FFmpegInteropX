#ifdef UWP
#include "VideoAdjustmentsConfiguration.h"
#include "VideoAdjustmentsConfiguration.g.cpp"

#include "VideoAdjustmentsEffect.h"

namespace winrt::FFmpegInteropX::VideoEffects::implementation
{
    float VideoAdjustmentsConfiguration::Brightness()
    {
        return _Brightness;
    }
    void VideoAdjustmentsConfiguration::Brightness(float value)
    {
        _Brightness = value;
    }
    float VideoAdjustmentsConfiguration::Contrast()
    {
        return _Contrast;
    }
    void VideoAdjustmentsConfiguration::Contrast(float value)
    {
        _Contrast = value;
    }
    float VideoAdjustmentsConfiguration::Saturation()
    {
        return _Saturation;
    }
    void VideoAdjustmentsConfiguration::Saturation(float value)
    {
        _Saturation = value;
    }
    float VideoAdjustmentsConfiguration::Temperature()
    {
        return _Temperature;
    }
    void VideoAdjustmentsConfiguration::Temperature(float value)
    {
        if (value < -1 || value > 1)
        {
            throw_hresult(E_INVALIDARG);
        }
        _Temperature = value;
    }
    float VideoAdjustmentsConfiguration::Tint()
    {
        return _Tint;
    }
    void VideoAdjustmentsConfiguration::Tint(float value)
    {
        if (value < -1 || value > 1)
        {
            throw_hresult(E_INVALIDARG);
        }
        _Tint = value;
    }
    float VideoAdjustmentsConfiguration::Sharpness()
    {
        return _Sharpness;
    }
    void VideoAdjustmentsConfiguration::Sharpness(float value)
    {
        if (value < 0 || value > 10)
        {
            throw_hresult(E_INVALIDARG);
        }
        _Sharpness = value;
    }
    float VideoAdjustmentsConfiguration::SharpnessThreshold()
    {
        return _SharpnessThreshold;
    }
    void VideoAdjustmentsConfiguration::SharpnessThreshold(float value)
    {
        if (value < 0 || value > 10)
        {
            throw_hresult(E_INVALIDARG);
        }
        _SharpnessThreshold = value;
    }
    void VideoAdjustmentsConfiguration::AddVideoEffect(winrt::Windows::Media::Playback::MediaPlayer const& player)
    {
        auto set = winrt::Windows::Foundation::Collections::PropertySet();
        set.Insert(L"config", *this);
        player.AddVideoEffect(winrt::name_of<winrt::FFmpegInteropX::VideoEffects::VideoAdjustmentsEffect>(), true, set);
    }
    void VideoAdjustmentsConfiguration::AddVideoEffect(winrt::Windows::Media::Playback::MediaPlayer const& player, bool isOptional)
    {
        auto set = winrt::Windows::Foundation::Collections::PropertySet();
        set.Insert(L"config", *this);
        player.AddVideoEffect(winrt::name_of<winrt::FFmpegInteropX::VideoEffects::VideoAdjustmentsEffect>(), isOptional, set);
    }
}
#endif
