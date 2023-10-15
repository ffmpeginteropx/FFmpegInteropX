#pragma once
#include "pch.h"
#include "MediaSourceConfig.g.h"
#include "ConfigurationCommon.h"
#include "AudioConfig.h"
#include "GeneralConfig.h"
#include "SubtitlesConfig.h"
#include "VideoConfig.h"

using namespace winrt::Windows::Foundation::Collections;
// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropX::implementation
{
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Windows::Foundation::Collections;
    using namespace winrt::Windows::Media::Core;

    struct MediaSourceConfig : MediaSourceConfigT<MediaSourceConfig>
    {
        ///<summary>Additional options to use when creating the ffmpeg AVFormatContext.</summary>
        PROPERTY_CONST(FFmpegOptions, PropertySet, PropertySet());

        ///<summary>Automatically extend the duration of the MediaStreamSource, if the file unexpectedly contains additional data.</summary>
        PROPERTY(AutoExtendDuration, bool, true);

        ///<summary> Audio settings: decoder settings,stereo downmix, outputs, maximum decoder threads.</summary>
        PROPERTY_CONST(Audio, winrt::FFmpegInteropX::AudioConfig, winrt::make<AudioConfig>());

        ///<summary> General settings: fast seek, stream buffering, max supported playback rate.</summary>
        PROPERTY_CONST(General, winrt::FFmpegInteropX::GeneralConfig, winrt::make<GeneralConfig>());

        ///<summary> Subtitles settings: encoding, style overrides, delays, minimum duration, default delay, localisation.</summary>
        PROPERTY_CONST(Subtitles, winrt::FFmpegInteropX::SubtitlesConfig, winrt::make<SubtitlesConfig>());

        ///<summary> Video settings: decoder settings, hardware acceleration, outputs, maximum decoder threads, HDR.</summary>
        PROPERTY_CONST(Video, winrt::FFmpegInteropX::VideoConfig, winrt::make<VideoConfig>());

    public:
        //internal:
        bool IsFrameGrabber;
        /*Internal use:determines if a FFmpegInteropInstance is in external subtitle parser mode. This mode is used to parse files which contain only subtitle streams*/
        bool IsExternalSubtitleParser;

        /*Used to pass additional, specific options to external sub parsers*/
        PropertySet AdditionalFFmpegSubtitleOptions = {nullptr};

  
    };
}
namespace winrt::FFmpegInteropX::factory_implementation
{
    struct MediaSourceConfig : MediaSourceConfigT<MediaSourceConfig, implementation::MediaSourceConfig>
    {
    };
}
