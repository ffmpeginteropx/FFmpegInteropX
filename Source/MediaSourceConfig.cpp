#include "pch.h"
#include "MediaSourceConfig.h"
#include "MediaSourceConfig.g.cpp"
#include "winrt/FFmpegInteropX.h"

using namespace winrt::Windows::Foundation::Collections;
using namespace winrt::Windows::Media::Core;


// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropX::implementation
{
    MediaSourceConfig::MediaSourceConfig()
    {
        m_PassthroughAudioMP3 = false;
        m_PassthroughAudioAAC = false;

        m_VideoDecoderMode = VideoDecoderMode::Automatic;

        m_SystemDecoderH264MaxProfile = FF_PROFILE_H264_HIGH;
        m_SystemDecoderH264MaxLevel = 41;
        m_SystemDecoderHEVCMaxProfile = FF_PROFILE_HEVC_MAIN_10;
        m_SystemDecoderHEVCMaxLevel = -1;
        m_VideoOutputAllowIyuv = false;
        m_VideoOutputAllow10bit = true;
        m_VideoOutputAllowBgra8 = false;
        m_VideoOutputAllowNv12 = true;

        m_SkipErrors = 50;
        m_MaxAudioThreads = 2;

        m_MaxSupportedPlaybackRate = 4.0;
        m_StreamBufferSize = 16384;

        m_FFmpegOptions = PropertySet();

        m_DefaultBufferTime = TimeSpan{ 0 };
        m_DefaultBufferTimeUri = TimeSpan{ 50000000 };

        m_AutoSelectForcedSubtitles = true;
        m_OverrideSubtitleStyles = false;

        m_SubtitleRegion = TimedTextRegion();

        TimedTextSize extent;
        extent.Unit = TimedTextUnit::Percentage;
        extent.Width = 100;
        extent.Height = 88;
        m_SubtitleRegion.Extent(extent);
        TimedTextPoint position;
        position.Unit = TimedTextUnit::Pixels;
        position.X = 0;
        position.Y = 0;
        m_SubtitleRegion.Position(position);
        m_SubtitleRegion.DisplayAlignment(TimedTextDisplayAlignment::After);
        m_SubtitleRegion.Background(winrt::Windows::UI::Colors::Transparent());
        m_SubtitleRegion.ScrollMode(TimedTextScrollMode::Rollup);
        m_SubtitleRegion.TextWrapping(TimedTextWrapping::Wrap);
        m_SubtitleRegion.WritingMode(TimedTextWritingMode::LeftRightTopBottom);
        m_SubtitleRegion.IsOverflowClipped(false);
        m_SubtitleRegion.ZIndex(0);
        TimedTextDouble LineHeight;
        LineHeight.Unit = TimedTextUnit::Percentage;
        LineHeight.Value = 100;
        m_SubtitleRegion.LineHeight(LineHeight);
        TimedTextPadding padding;
        padding.Unit = TimedTextUnit::Percentage;
        padding.Start = 0;
        padding.After = 0;
        padding.Before = 0;
        padding.End = 0;
        m_SubtitleRegion.Padding(padding);
        m_SubtitleRegion.Name(L"");

        m_SubtitleStyle = TimedTextStyle();

        m_SubtitleStyle.FontFamily(L"default");
        TimedTextDouble fontSize;
        fontSize.Unit = TimedTextUnit::Percentage;
        fontSize.Value = 100;
        m_SubtitleStyle.FontSize(fontSize);
        m_SubtitleStyle.LineAlignment(TimedTextLineAlignment::Center);
        m_SubtitleStyle.FontStyle(TimedTextFontStyle::Normal);
        m_SubtitleStyle.FontWeight(TimedTextWeight::Normal);
        m_SubtitleStyle.Foreground(winrt::Windows::UI::Colors::White());
        m_SubtitleStyle.Background(Windows::UI::Colors::Transparent());
        //OutlineRadius = new TimedTextDouble { Unit = TimedTextUnit.Percentage, Value = 10 },
        TimedTextDouble outlineThickness;
        outlineThickness.Unit = TimedTextUnit::Percentage;
        outlineThickness.Value = 4.5;
        m_SubtitleStyle.OutlineThickness(outlineThickness);
        m_SubtitleStyle.FlowDirection(TimedTextFlowDirection::LeftToRight);
        m_SubtitleStyle.OutlineColor(winrt::Windows::UI::Color{ 0x80, 0, 0, 0 });

        m_AutoCorrectAnsiSubtitles = true;
        AnsiSubtitleEncoding(CharacterEncoding::GetSystemDefault());

        m_FastSeek = false;
        m_FastSeekCleanAudio = true;
        m_FastSeekSmartStreamSwitching = true;

        m_DefaultAudioStreamName = L"Audio Stream";
        m_DefaultSubtitleStreamName = L"Subtitle";
        m_DefaultExternalSubtitleStreamName = L"External Subtitle";

        m_AttachmentCacheFolderName = L"FFmpegAttachmentCache";
        m_UseEmbeddedSubtitleFonts = true;

        m_MinimumSubtitleDuration = TimeSpan{ 0 };
        m_AdditionalSubtitleDuration = TimeSpan{ 0 };
        m_PreventModifiedSubtitleDurationOverlap = true;
    }


    bool MediaSourceConfig::PassthroughAudioMP3()
    {
        return m_PassthroughAudioMP3;
    }

    void MediaSourceConfig::PassthroughAudioMP3(bool value)
    {
        m_PassthroughAudioMP3 = value;
    }

    bool MediaSourceConfig::PassthroughAudioAAC()
    {
        return m_PassthroughAudioAAC;
    }

    void MediaSourceConfig::PassthroughAudioAAC(bool value)
    {
        m_PassthroughAudioAAC = value;
    }

    FFmpegInteropX::VideoDecoderMode MediaSourceConfig::VideoDecoderMode()
    {
        return m_VideoDecoderMode;
    }

    void MediaSourceConfig::VideoDecoderMode(FFmpegInteropX::VideoDecoderMode const& value)
    {
        m_VideoDecoderMode = value;
    }
    int32_t MediaSourceConfig::SystemDecoderH264MaxProfile()
    {
        return m_SystemDecoderH264MaxProfile;
    }

    void MediaSourceConfig::SystemDecoderH264MaxProfile(int32_t value)
    {
        m_SystemDecoderH264MaxProfile = value;
    }

    int32_t MediaSourceConfig::SystemDecoderH264MaxLevel()
    {
        return m_SystemDecoderH264MaxLevel;
    }

    void MediaSourceConfig::SystemDecoderH264MaxLevel(int32_t value)
    {
        m_SystemDecoderH264MaxLevel = value;
    }

    int32_t MediaSourceConfig::SystemDecoderHEVCMaxProfile()
    {
        return m_SystemDecoderHEVCMaxProfile;
    }

    void MediaSourceConfig::SystemDecoderHEVCMaxProfile(int32_t value)
    {
        m_SystemDecoderHEVCMaxProfile = value;
    }

    int32_t MediaSourceConfig::SystemDecoderHEVCMaxLevel()
    {
        return m_SystemDecoderHEVCMaxLevel;
    }

    void MediaSourceConfig::SystemDecoderHEVCMaxLevel(int32_t value)
    {
        m_SystemDecoderHEVCMaxLevel = value;
    }

    bool MediaSourceConfig::VideoOutputAllowIyuv()
    {
        return m_VideoOutputAllowIyuv;
    }

    void MediaSourceConfig::VideoOutputAllowIyuv(bool value)
    {
        m_VideoOutputAllowIyuv = value;
    }

    bool MediaSourceConfig::VideoOutputAllow10bit()
    {
        return m_VideoOutputAllow10bit;
    }

    void MediaSourceConfig::VideoOutputAllow10bit(bool value)
    {
        m_VideoOutputAllow10bit = value;
    }

    bool MediaSourceConfig::VideoOutputAllowBgra8()
    {
        return m_VideoOutputAllowBgra8;
    }

    void MediaSourceConfig::VideoOutputAllowBgra8(bool value)
    {
        m_VideoOutputAllowBgra8 = value;
    }

    bool MediaSourceConfig::VideoOutputAllowNv12()
    {
        return m_VideoOutputAllowNv12;
    }

    void MediaSourceConfig::VideoOutputAllowNv12(bool value)
    {
        m_VideoOutputAllowNv12 = value;
    }

    uint32_t MediaSourceConfig::SkipErrors()
    {
        return m_SkipErrors;
    }

    void MediaSourceConfig::SkipErrors(uint32_t value)
    {
        m_SkipErrors = value;
    }

    uint32_t MediaSourceConfig::MaxVideoThreads()
    {
        return m_MaxVideoThreads;
    }

    void MediaSourceConfig::MaxVideoThreads(uint32_t value)
    {
        m_MaxVideoThreads = value;
    }

    uint32_t MediaSourceConfig::MaxAudioThreads()
    {
        return m_MaxAudioThreads;
    }

    void MediaSourceConfig::MaxAudioThreads(uint32_t value)
    {
        m_MaxAudioThreads = value;
    }

    double MediaSourceConfig::MaxSupportedPlaybackRate()
    {
        return m_MaxSupportedPlaybackRate;
    }

    void MediaSourceConfig::MaxSupportedPlaybackRate(double value)
    {
        m_MaxSupportedPlaybackRate = value;
    }

    uint32_t MediaSourceConfig::StreamBufferSize()
    {
        return m_StreamBufferSize;
    }

    void MediaSourceConfig::StreamBufferSize(uint32_t value)
    {
        m_StreamBufferSize = value;
    }

    Windows::Foundation::Collections::PropertySet MediaSourceConfig::FFmpegOptions()
    {
        return m_FFmpegOptions;
    }

    void MediaSourceConfig::FFmpegOptions(Windows::Foundation::Collections::PropertySet const& value)
    {
        m_FFmpegOptions = value;
    }

    Windows::Foundation::TimeSpan MediaSourceConfig::DefaultBufferTime()
    {
        return m_DefaultBufferTime;
    }

    void MediaSourceConfig::DefaultBufferTime(Windows::Foundation::TimeSpan const& value)
    {
        m_DefaultBufferTime = value;
    }

    Windows::Foundation::TimeSpan MediaSourceConfig::DefaultBufferTimeUri()
    {
        return m_DefaultBufferTimeUri;
    }

    void MediaSourceConfig::DefaultBufferTimeUri(Windows::Foundation::TimeSpan const& value)
    {
        m_DefaultBufferTimeUri = value;
    }

    bool MediaSourceConfig::AutoSelectForcedSubtitles()
    {
        return m_AutoSelectForcedSubtitles;
    }

    void MediaSourceConfig::AutoSelectForcedSubtitles(bool value)
    {
        m_AutoSelectForcedSubtitles = value;
    }

    bool MediaSourceConfig::OverrideSubtitleStyles()
    {
        return m_OverrideSubtitleStyles;
    }

    void MediaSourceConfig::OverrideSubtitleStyles(bool value)
    {
        m_OverrideSubtitleStyles = value;
    }

    Windows::Media::Core::TimedTextRegion MediaSourceConfig::SubtitleRegion()
    {
        return m_SubtitleRegion;
    }

    void MediaSourceConfig::SubtitleRegion(Windows::Media::Core::TimedTextRegion const& value)
    {
        m_SubtitleRegion = value;
    }

    Windows::Media::Core::TimedTextStyle MediaSourceConfig::SubtitleStyle()
    {
        return m_SubtitleStyle;
    }

    void MediaSourceConfig::SubtitleStyle(Windows::Media::Core::TimedTextStyle const& value)
    {
        m_SubtitleStyle = value;
    }

    bool MediaSourceConfig::AutoCorrectAnsiSubtitles()
    {
        return m_AutoCorrectAnsiSubtitles;
    }

    void MediaSourceConfig::AutoCorrectAnsiSubtitles(bool value)
    {
        m_AutoCorrectAnsiSubtitles = value;
    }

    FFmpegInteropX::CharacterEncoding MediaSourceConfig::AnsiSubtitleEncoding()
    {
        return m_CharacterEncoding;
    }

    void MediaSourceConfig::AnsiSubtitleEncoding(FFmpegInteropX::CharacterEncoding const& value)
    {
        if (value == nullptr)
            throw_hresult(E_INVALIDARG);
        m_CharacterEncoding = value;
    }

    Windows::Foundation::TimeSpan MediaSourceConfig::DefaultSubtitleDelay()
    {
        return m_DefaultSubtitleDelay;
    }

    void MediaSourceConfig::DefaultSubtitleDelay(Windows::Foundation::TimeSpan const& value)
    {
        m_DefaultSubtitleDelay = value;
    }

    bool MediaSourceConfig::FastSeek()
    {
        return m_FastSeek;
    }

    void MediaSourceConfig::FastSeek(bool value)
    {
        m_FastSeek = value;
    }

    bool MediaSourceConfig::FastSeekCleanAudio()
    {
        return m_FastSeekCleanAudio;
    }

    void MediaSourceConfig::FastSeekCleanAudio(bool value)
    {
        m_FastSeekCleanAudio = value;
    }

    bool MediaSourceConfig::FastSeekSmartStreamSwitching()
    {
        return m_FastSeekSmartStreamSwitching;
    }

    void MediaSourceConfig::FastSeekSmartStreamSwitching(bool value)
    {
        m_FastSeekSmartStreamSwitching = value;
    }

    hstring MediaSourceConfig::DefaultAudioStreamName()
    {
        return m_DefaultAudioStreamName;
    }

    void MediaSourceConfig::DefaultAudioStreamName(hstring const& value)
    {
        m_DefaultAudioStreamName = value;
    }

    hstring MediaSourceConfig::DefaultSubtitleStreamName()
    {
        return m_DefaultSubtitleStreamName;
    }

    void MediaSourceConfig::DefaultSubtitleStreamName(hstring const& value)
    {
        m_DefaultSubtitleStreamName = value;
    }

    hstring MediaSourceConfig::DefaultExternalSubtitleStreamName()
    {
        return m_DefaultExternalSubtitleStreamName;
    }

    void MediaSourceConfig::DefaultExternalSubtitleStreamName(hstring const& value)
    {
        m_DefaultExternalSubtitleStreamName = value;
    }

    bool MediaSourceConfig::UseEmbeddedSubtitleFonts()
    {
        return m_UseEmbeddedSubtitleFonts;
    }

    void MediaSourceConfig::UseEmbeddedSubtitleFonts(bool value)
    {
        m_UseEmbeddedSubtitleFonts = value;
    }

    hstring MediaSourceConfig::AttachmentCacheFolderName()
    {
        return m_AttachmentCacheFolderName;
    }

    void MediaSourceConfig::AttachmentCacheFolderName(hstring const& value)
    {
        m_AttachmentCacheFolderName = value;
    }

    Windows::Foundation::TimeSpan MediaSourceConfig::MinimumSubtitleDuration()
    {
        return m_MinimumSubtitleDuration;
    }

    void MediaSourceConfig::MinimumSubtitleDuration(Windows::Foundation::TimeSpan const& value)
    {
        m_MinimumSubtitleDuration = value;
    }

    Windows::Foundation::TimeSpan MediaSourceConfig::AdditionalSubtitleDuration()
    {
        return m_AdditionalSubtitleDuration;
    }

    void MediaSourceConfig::AdditionalSubtitleDuration(Windows::Foundation::TimeSpan const& value)
    {
        m_AdditionalSubtitleDuration = value;
    }

    bool MediaSourceConfig::PreventModifiedSubtitleDurationOverlap()
    {
        return m_PreventModifiedSubtitleDurationOverlap;
    }

    void MediaSourceConfig::PreventModifiedSubtitleDurationOverlap(bool value)
    {
        m_PreventModifiedSubtitleDurationOverlap = value;
    }

    hstring MediaSourceConfig::FFmpegVideoFilters()
    {
        return m_FFmpegVideoFilters;
    }

    void MediaSourceConfig::FFmpegVideoFilters(hstring const& value)
    {
        m_FFmpegVideoFilters = value;
    }

    hstring MediaSourceConfig::FFmpegAudioFilters()
    {
        return m_FFmpegAudioFilters;
    }

    void MediaSourceConfig::FFmpegAudioFilters(hstring const& value)
    {
        m_FFmpegAudioFilters = value;
    }

    bool MediaSourceConfig::DownmixAudioStreamsToStereo()
    {
        return m_DownmixAudioStreamsToStereo;
    }

    void MediaSourceConfig::DownmixAudioStreamsToStereo(bool value)
    {
        m_DownmixAudioStreamsToStereo = value;
    }

    winrt::FFmpegInteropX::IVideoFrameProcessor MediaSourceConfig::VideoEffectProcessor()
    {
        return m_videoEffectsProcessor;
    }

    void MediaSourceConfig::VideoEffectProcessor(winrt::FFmpegInteropX::IVideoFrameProcessor const& value)
    {
        m_videoEffectsProcessor = value;
    }
}
