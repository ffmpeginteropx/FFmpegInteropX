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
        MediaSourceConfig();

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
        winrt::Windows::Foundation::TimeSpan DefaultBufferTime();
        void DefaultBufferTime(winrt::Windows::Foundation::TimeSpan const& value);
        winrt::Windows::Foundation::TimeSpan DefaultBufferTimeUri();
        void DefaultBufferTimeUri(winrt::Windows::Foundation::TimeSpan const& value);
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
        winrt::Windows::Foundation::TimeSpan DefaultSubtitleDelay();
        void DefaultSubtitleDelay(winrt::Windows::Foundation::TimeSpan const& value);
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
        winrt::Windows::Foundation::TimeSpan MinimumSubtitleDuration();
        void MinimumSubtitleDuration(winrt::Windows::Foundation::TimeSpan const& value);
        winrt::Windows::Foundation::TimeSpan AdditionalSubtitleDuration();
        void AdditionalSubtitleDuration(winrt::Windows::Foundation::TimeSpan const& value);
        bool PreventModifiedSubtitleDurationOverlap();
        void PreventModifiedSubtitleDurationOverlap(bool value);
        hstring FFmpegVideoFilters();
        void FFmpegVideoFilters(hstring const& value);
        hstring FFmpegAudioFilters();
        void FFmpegAudioFilters(hstring const& value);
        bool DownmixAudioStreamsToStereo();
        void DownmixAudioStreamsToStereo(bool value);
    public:
        //internal:
        bool IsFrameGrabber;
        /*Internal use:determines if a FFmpegInteropInstance is in external subtitle parser mode. This mode is used to parse files which contain only subtitle streams*/
        bool IsExternalSubtitleParser;

        /*Used to pass additional, specific options to external sub parsers*/
        PropertySet AdditionalFFmpegSubtitleOptions = {};

    private:

        ///<summary>Enable passthrough for MP3 audio to system decoder.</summary>
        ///<remarks>This could allow hardware decoding on some platforms (e.g. Windows Phone).</remarks>
        bool m_PassthroughAudioMP3 = false;

        ///<summary>Enable passthrough for AAC audio to system decoder.</summary>
        ///<remarks>This could allow hardware decoding on some platforms (e.g. Windows Phone).</remarks>
        bool m_PassthroughAudioAAC = false;

        ///<summary>Sets the video decoder mode. Default is AutoDetection.</summary>
        winrt::FFmpegInteropXWinUI::VideoDecoderMode m_VideoDecoderMode;

        ///<summary>Max profile allowed for H264 system decoder. Default: High Profile (100). See FF_PROFILE_H264_* values.</summary>
        int m_SystemDecoderH264MaxProfile = 0;

        ///<summary>Max level allowed for H264 system decoder. Default: Level 4.1 (41). Use -1 to disable level check.</summary>
        ///<remarks>Most H264 HW decoders only support Level 4.1, so this is the default.</remarks>
        int m_SystemDecoderH264MaxLevel = 0;


        ///<summary>Max profile allowed for HEVC system decoder. Default: High10 Profile (2). See FF_PROFILE_HEVC_* values.</summary>
        int m_SystemDecoderHEVCMaxProfile = 0;

        ///<summary>Max level allowed for HEVC system decoder. Default: Disabled (-1).</summary>
        ///<remarks>Encoded as: 30*Major + 3*Minor. So Level 6.0 = 30*6 = 180, 5.1 = 30*5 + 3*1 = 163, 4.1 = 123.
        ///Many HEVC HW decoders support even very high levels, so we disable the check by default.</remarks>
        int m_SystemDecoderHEVCMaxLevel = 0;
        ///<summary>Allow video output in IYuv format.</summary>
        bool m_VideoOutputAllowIyuv = false;

        ///<summary>Allow video output in 10bit formats.</summary>
        bool m_VideoOutputAllow10bit = false;

        ///<summary>Allow video output in BGRA format - required for video transparency.</summary>
        bool m_VideoOutputAllowBgra8 = false;

        ///<summary>Allow video output in NV12 format.</summary>
        bool m_VideoOutputAllowNv12 = false;


        ///<summary>The maximum number of broken frames to skipp in a stream before stopping decoding.</summary>
        unsigned int m_SkipErrors = 0;

        ///<summary>The maximum number of video decoding threads.</summary>
        unsigned int m_MaxVideoThreads = 0;

        ///<summary>The maximum number of audio decoding threads.</summary>
        unsigned int m_MaxAudioThreads = 0;

        ///<summary>The maximum supported playback rate. This is set on the media stream source itself. 
        /// This does not modify what the transport control default UI shows as available playback speeds. Custom UI is necessary!</summary>
        double m_MaxSupportedPlaybackRate = 0.0;

        ///<summary>The buffer size in bytes to use for IRandomAccessStream sources.</summary>
        unsigned int m_StreamBufferSize = 0;

        ///<summary>Additional options to use when creating the ffmpeg AVFormatContext.</summary>
        winrt::Windows::Foundation::Collections::PropertySet m_FFmpegOptions = {};


        ///<summary>The default BufferTime that gets assigned to the MediaStreamSource for IRandomAccessStream sources.</summary>
        ///<remarks>A value of 0 is recommended for local files, to avoid framework bugs and unneccessary memory consumption.</remarks>
        winrt::Windows::Foundation::TimeSpan m_DefaultBufferTime{};

        ///<summary>The default BufferTime that gets assigned to the MediaStreamSource for URI sources.</summary>
        ///<remarks>Default is 5 seconds. You might want to use higher values, especially for DASH stream sources.</remarks>
        winrt::Windows::Foundation::TimeSpan m_DefaultBufferTimeUri{};


        ///<summary>Automatically select subtitles when they have the 'forced' flag set.</summary>
        bool m_AutoSelectForcedSubtitles = false;

        ///<summary>Use SubtitleRegion and SubtitleStyle from config class, even if custom styles are defined for a subtitle.</summary>
        bool m_OverrideSubtitleStyles = false;

        ///<summary>Default region to use for subtitles.</summary>
        winrt::Windows::Media::Core::TimedTextRegion m_SubtitleRegion = {};

        ///<summary>Default style to use for subtitles.</summary>
        winrt::Windows::Media::Core::TimedTextStyle m_SubtitleStyle = {};

        ///<summary>Enable conversion of ANSI encoded subtitles to UTF-8.</summary>
        bool m_AutoCorrectAnsiSubtitles = false;
        ///<summary>The subtitle delay will be initially applied to all subtitle tracks.
            ///Use SetSubtitleDelay() on the FFmpegMediaSource instance if you want to change the delay during playback.</summary>
        winrt::Windows::Foundation::TimeSpan m_DefaultSubtitleDelay{};

        /// <summary>FFmpegMediaSource will seek to the closest video keyframe, if set to true.</summary>
        /// <remarks>
        /// For FastSeek to work, you must use the MediaPlayer for playback, and assign
        /// MediaPlayer.PlaybackSession to the FFmpegMediaSource.PlaybackSession .
        /// </remarks>
        bool m_FastSeek = false;

        ///<summary>Ensure that audio plays without artifacts after fast seeking.</summary>
        ///<remarks>This will slightly reduce the speed of fast seeking. Enabled by default.</remarks>
        bool m_FastSeekCleanAudio = false;

        ///<summary>Try to improve stream switching times when FastSeek is enabled.</summary>
        bool m_FastSeekSmartStreamSwitching = false;

        ///<summary>The default name to use for audio streams.</summary>
        hstring m_DefaultAudioStreamName{};

        ///<summary>The default name to use for subtitle streams.</summary>
        hstring m_DefaultSubtitleStreamName{};

        ///<summary>The default name to use for external subtitle streams.</summary>
        hstring m_DefaultExternalSubtitleStreamName{};

        ///<summary>Use subtitle font files that are embedded in the media file.</summary>
        bool m_UseEmbeddedSubtitleFonts = false;

        ///<summary>The folder where attachments such as fonts are stored (inside the app's temp folder).</summary>
        hstring m_AttachmentCacheFolderName{};

        ///<summary>The minimum amount of time a subtitle should be shown. Default is 0.</summary>
        winrt::Windows::Foundation::TimeSpan m_MinimumSubtitleDuration{};

        ///<summary>Each subtitle's duration is extended by this amount. Default is 0.</summary>
        winrt::Windows::Foundation::TimeSpan m_AdditionalSubtitleDuration{};

        ///<summary>Try to prevent overlapping subtitles when extending durations.</summary>
        bool m_PreventModifiedSubtitleDurationOverlap = false;


        ///<summary>Initial FFmpeg video filters. Might be changed later through FFmpegMediaSource.SetFFmpegVideoFilters().</summary>
        ///<remarks>Using FFmpeg video filters will degrade playback performance, since they run on the CPU and not on the GPU.</remarks>
        hstring m_FFmpegVideoFilters{};

        ///<summary>Initial FFmpeg audio filters. Might be changed later through FFmpegMediaSource.SetFFmpegAudioFilters().</summary>
        hstring m_FFmpegAudioFilters{};

        ///<summary>Downmix multi-channel audio streams to stereo format.</summary>
        bool m_DownmixAudioStreamsToStereo = false;
        CharacterEncoding m_CharacterEncoding = {};
    };
}
namespace winrt::FFmpegInteropXWinUI::factory_implementation
{
    struct MediaSourceConfig : MediaSourceConfigT<MediaSourceConfig, implementation::MediaSourceConfig>
    {
    };
}
