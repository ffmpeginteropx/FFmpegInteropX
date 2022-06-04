#pragma once
#include <pch.h>
#include "MediaSourceConfig.g.h"
using namespace winrt::Windows::Foundation::Collections;
// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropXWinUI::implementation
{
    struct MediaSourceConfig : MediaSourceConfigT<MediaSourceConfig>
    {
    public:
        MediaSourceConfig() = default;

        bool PassthroughAudioMP3();
        void PassthroughAudioMP3(bool value);
        bool PassthroughAudioAAC();
        void PassthroughAudioAAC(bool value);
        FFmpegInteropXWinUI::VideoDecoderMode VideoDecoderMode();
        void VideoDecoderMode(FFmpegInteropXWinUI::VideoDecoderMode const& value);
        int32_t SystemDecoderH264MaxProfile();
        void SystemDecoderH264MaxProfile(int32_t value);
        int32_t SystemDecoderH264MaxLevel();
        void SystemDecoderH264MaxLevel(int32_t value);
        int32_t SystemDecoderHEVCMaxProfile();
        void SystemDecoderHEVCMaxProfile(int32_t value);
        int32_t SystemDecoderHEVCMaxLevel();
        void SystemDecoderHEVCMaxLevel(int32_t value);
        bool VideoOutputAllowIyuv();
        void VideoOutputAllowIyuv(bool value);
        bool VideoOutputAllow10bit();
        void VideoOutputAllow10bit(bool value);
        bool VideoOutputAllowBgra8();
        void VideoOutputAllowBgra8(bool value);
        bool VideoOutputAllowNv12();
        void VideoOutputAllowNv12(bool value);
        uint32_t SkipErrors();
        void SkipErrors(uint32_t value);
        uint32_t MaxVideoThreads();
        void MaxVideoThreads(uint32_t value);
        uint32_t MaxAudioThreads();
        void MaxAudioThreads(uint32_t value);
        double MaxSupportedPlaybackRate();
        void MaxSupportedPlaybackRate(double value);
        uint32_t StreamBufferSize();
        void StreamBufferSize(uint32_t value);
        Windows::Foundation::Collections::PropertySet FFmpegOptions();
        void FFmpegOptions(Windows::Foundation::Collections::PropertySet const& value);
        Windows::Foundation::TimeSpan DefaultBufferTime();
        void DefaultBufferTime(Windows::Foundation::TimeSpan const& value);
        Windows::Foundation::TimeSpan DefaultBufferTimeUri();
        void DefaultBufferTimeUri(Windows::Foundation::TimeSpan const& value);
        bool AutoSelectForcedSubtitles();
        void AutoSelectForcedSubtitles(bool value);
        bool OverrideSubtitleStyles();
        void OverrideSubtitleStyles(bool value);
        Windows::Media::Core::TimedTextRegion SubtitleRegion();
        void SubtitleRegion(Windows::Media::Core::TimedTextRegion const& value);
        Windows::Media::Core::TimedTextStyle SubtitleStyle();
        void SubtitleStyle(Windows::Media::Core::TimedTextStyle const& value);
        bool AutoCorrectAnsiSubtitles();
        void AutoCorrectAnsiSubtitles(bool value);
        FFmpegInteropXWinUI::CharacterEncoding AnsiSubtitleEncoding();
        void AnsiSubtitleEncoding(FFmpegInteropXWinUI::CharacterEncoding const& value);
        Windows::Foundation::TimeSpan DefaultSubtitleDelay();
        void DefaultSubtitleDelay(Windows::Foundation::TimeSpan const& value);
        bool FastSeek();
        void FastSeek(bool value);
        bool FastSeekCleanAudio();
        void FastSeekCleanAudio(bool value);
        bool FastSeekSmartStreamSwitching();
        void FastSeekSmartStreamSwitching(bool value);
        hstring DefaultAudioStreamName();
        void DefaultAudioStreamName(hstring const& value);
        hstring DefaultSubtitleStreamName();
        void DefaultSubtitleStreamName(hstring const& value);
        hstring DefaultExternalSubtitleStreamName();
        void DefaultExternalSubtitleStreamName(hstring const& value);
        bool UseEmbeddedSubtitleFonts();
        void UseEmbeddedSubtitleFonts(bool value);
        hstring AttachmentCacheFolderName();
        void AttachmentCacheFolderName(hstring const& value);
        Windows::Foundation::TimeSpan MinimumSubtitleDuration();
        void MinimumSubtitleDuration(Windows::Foundation::TimeSpan const& value);
        Windows::Foundation::TimeSpan AdditionalSubtitleDuration();
        void AdditionalSubtitleDuration(Windows::Foundation::TimeSpan const& value);
        bool PreventModifiedSubtitleDurationOverlap();
        void PreventModifiedSubtitleDurationOverlap(bool value);
        hstring FFmpegVideoFilters();
        void FFmpegVideoFilters(hstring const& value);
        hstring FFmpegAudioFilters();
        void FFmpegAudioFilters(hstring const& value);
        bool DownmixAudioStreamsToStereo();
        void DownmixAudioStreamsToStereo(bool value);

        //internal:
        bool IsFrameGrabber;
        /*Internal use:determines if a FFmpegInteropInstance is in external subtitle parser mode. This mode is used to parse files which contain only subtitle streams*/
        bool IsExternalSubtitleParser;

        /*Used to pass additional, specific options to external sub parsers*/
        PropertySet AdditionalFFmpegSubtitleOptions;
    };
}
namespace winrt::FFmpegInteropXWinUI::factory_implementation
{
    struct MediaSourceConfig : MediaSourceConfigT<MediaSourceConfig, implementation::MediaSourceConfig>
    {
    };
}
