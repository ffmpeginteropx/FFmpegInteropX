#pragma once

extern "C"
{
#include "libavcodec/avcodec.h"
}

#include "Enumerations.h"
#include "CharacterEncoding.h"
#include "TimeSpanHelpers.h"


namespace FFmpegInteropX
{
    using namespace Platform;
    using namespace Windows::Foundation;
    using namespace Windows::Foundation::Collections;
    using namespace Windows::Media::Core;

    namespace WFM = Windows::Foundation::Metadata;

    ///<summary>This class allows configuring the FFmpegMediaSource instance.</summary>
    public ref class MediaSourceConfig sealed
    {
    public:
        MediaSourceConfig()
        {
            PassthroughAudioMP3 = false;
            PassthroughAudioAAC = false;

            VideoDecoderMode = FFmpegInteropX::VideoDecoderMode::Automatic;

            SystemDecoderH264MaxProfile = FF_PROFILE_H264_HIGH;
            SystemDecoderH264MaxLevel = 41;
            SystemDecoderHEVCMaxProfile = FF_PROFILE_HEVC_MAIN_10;
            SystemDecoderHEVCMaxLevel = -1;
            VideoOutputAllowIyuv = false;
            VideoOutputAllow10bit = true;
            VideoOutputAllowBgra8 = false;
            VideoOutputAllowNv12 = true;

            SkipErrors = 50;
            MaxAudioThreads = 2;

            MaxSupportedPlaybackRate = 4.0;
            StreamBufferSize = 16384;

            FFmpegOptions = ref new PropertySet();

            DefaultBufferTime = TimeSpan{ 0 };
            DefaultBufferTimeUri = TimeSpan{ 50000000 };

            AutoSelectForcedSubtitles = true;
            OverrideSubtitleStyles = false;

            SubtitleRegion = ref new TimedTextRegion();

            TimedTextSize extent;
            extent.Unit = TimedTextUnit::Percentage;
            extent.Width = 100;
            extent.Height = 88;
            SubtitleRegion->Extent = extent;
            TimedTextPoint position;
            position.Unit = TimedTextUnit::Pixels;
            position.X = 0;
            position.Y = 0;
            SubtitleRegion->Position = position;
            SubtitleRegion->DisplayAlignment = TimedTextDisplayAlignment::After;
            SubtitleRegion->Background = Windows::UI::Colors::Transparent;
            SubtitleRegion->ScrollMode = TimedTextScrollMode::Rollup;
            SubtitleRegion->TextWrapping = TimedTextWrapping::Wrap;
            SubtitleRegion->WritingMode = TimedTextWritingMode::LeftRightTopBottom;
            SubtitleRegion->IsOverflowClipped = false;
            SubtitleRegion->ZIndex = 0;
            TimedTextDouble LineHeight;
            LineHeight.Unit = TimedTextUnit::Percentage;
            LineHeight.Value = 100;
            SubtitleRegion->LineHeight = LineHeight;
            TimedTextPadding padding;
            padding.Unit = TimedTextUnit::Percentage;
            padding.Start = 0;
            SubtitleRegion->Padding = padding;
            SubtitleRegion->Name = "";

            SubtitleStyle = ref new TimedTextStyle();

            SubtitleStyle->FontFamily = "default";
            TimedTextDouble fontSize;
            fontSize.Unit = TimedTextUnit::Pixels;
            fontSize.Value = 44;
            SubtitleStyle->FontSize = fontSize;
            SubtitleStyle->LineAlignment = TimedTextLineAlignment::Center;
            if (Windows::Foundation::Metadata::ApiInformation::IsPropertyPresent("Windows.Media.Core.TimedTextStyle", "FontStyle"))
            {
                SubtitleStyle->FontStyle = TimedTextFontStyle::Normal;
            }
            SubtitleStyle->FontWeight = TimedTextWeight::Normal;
            SubtitleStyle->Foreground = Windows::UI::Colors::White;
            SubtitleStyle->Background = Windows::UI::Colors::Transparent;
            //OutlineRadius = new TimedTextDouble { Unit = TimedTextUnit.Percentage, Value = 10 },
            TimedTextDouble outlineThickness;
            outlineThickness.Unit = TimedTextUnit::Percentage;
            outlineThickness.Value = 4.5;
            SubtitleStyle->OutlineThickness = outlineThickness;
            SubtitleStyle->FlowDirection = TimedTextFlowDirection::LeftToRight;
            SubtitleStyle->OutlineColor = Windows::UI::Color{ 0x80, 0, 0, 0 };

            AutoCorrectAnsiSubtitles = true;
            AnsiSubtitleEncoding = CharacterEncoding::GetSystemDefault();

            FastSeek = false;
            FastSeekCleanAudio = true;
            FastSeekSmartStreamSwitching = true;

            DefaultAudioStreamName = "Audio Stream";
            DefaultSubtitleStreamName = "Subtitle";
            DefaultExternalSubtitleStreamName = "External Subtitle";

            AttachmentCacheFolderName = "FFmpegAttachmentCache";
            UseEmbeddedSubtitleFonts = true;

            MinimumSubtitleDuration = TimeSpan{ 0 };
            AdditionalSubtitleDuration = TimeSpan{ 0 };
            PreventModifiedSubtitleDurationOverlap = true;
        };


        ///<summary>Enable passthrough for MP3 audio to system decoder.</summary>
        ///<remarks>This could allow hardware decoding on some platforms (e.g. Windows Phone).</remarks>
        property bool PassthroughAudioMP3;

        ///<summary>Enable passthrough for AAC audio to system decoder.</summary>
        ///<remarks>This could allow hardware decoding on some platforms (e.g. Windows Phone).</remarks>
        property bool PassthroughAudioAAC;

        ///<summary>Sets the video decoder mode. Default is AutoDetection.</summary>
        property FFmpegInteropX::VideoDecoderMode VideoDecoderMode;

        ///<summary>Max profile allowed for H264 system decoder. Default: High Profile (100). See FF_PROFILE_H264_* values.</summary>
        property int SystemDecoderH264MaxProfile;

        ///<summary>Max level allowed for H264 system decoder. Default: Level 4.1 (41). Use -1 to disable level check.</summary>
        ///<remarks>Most H264 HW decoders only support Level 4.1, so this is the default.</remarks>
        property int SystemDecoderH264MaxLevel;


        ///<summary>Max profile allowed for HEVC system decoder. Default: High10 Profile (2). See FF_PROFILE_HEVC_* values.</summary>
        property int SystemDecoderHEVCMaxProfile;

        ///<summary>Max level allowed for HEVC system decoder. Default: Disabled (-1).</summary>
        ///<remarks>Encoded as: 30*Major + 3*Minor. So Level 6.0 = 30*6 = 180, 5.1 = 30*5 + 3*1 = 163, 4.1 = 123.
        ///Many HEVC HW decoders support even very high levels, so we disable the check by default.</remarks>
        property int SystemDecoderHEVCMaxLevel;
        ///<summary>Allow video output in IYuv format.</summary>
        property bool VideoOutputAllowIyuv;

        ///<summary>Allow video output in 10bit formats.</summary>
        property bool VideoOutputAllow10bit;

        ///<summary>Allow video output in BGRA format - required for video transparency.</summary>
        property bool VideoOutputAllowBgra8;

        ///<summary>Allow video output in NV12 format.</summary>
        property bool VideoOutputAllowNv12;


        ///<summary>The maximum number of broken frames to skipp in a stream before stopping decoding.</summary>
        property unsigned int SkipErrors;

        ///<summary>The maximum number of video decoding threads.</summary>
        property unsigned int MaxVideoThreads;

        ///<summary>The maximum number of audio decoding threads.</summary>
        property unsigned int MaxAudioThreads;

        ///<summary>The maximum supported playback rate. This is set on the media stream source itself. 
        /// This does not modify what the transport control default UI shows as available playback speeds. Custom UI is necessary!</summary>
        property double MaxSupportedPlaybackRate;

        ///<summary>The buffer size in bytes to use for IRandomAccessStream sources.</summary>
        property unsigned int StreamBufferSize;

        ///<summary>Additional options to use when creating the ffmpeg AVFormatContext.</summary>
        property PropertySet^ FFmpegOptions;


        ///<summary>The default BufferTime that gets assigned to the MediaStreamSource for IRandomAccessStream sources.</summary>
        ///<remarks>A value of 0 is recommended for local files, to avoid framework bugs and unneccessary memory consumption.</remarks>
        property TimeSpan DefaultBufferTime;

        ///<summary>The default BufferTime that gets assigned to the MediaStreamSource for URI sources.</summary>
        ///<remarks>Default is 5 seconds. You might want to use higher values, especially for DASH stream sources.</remarks>
        property TimeSpan DefaultBufferTimeUri;


        ///<summary>Automatically select subtitles when they have the 'forced' flag set.</summary>
        property bool AutoSelectForcedSubtitles;

        ///<summary>Use SubtitleRegion and SubtitleStyle from config class, even if custom styles are defined for a subtitle.</summary>
        property bool OverrideSubtitleStyles;

        ///<summary>Default region to use for subtitles.</summary>
        property TimedTextRegion^ SubtitleRegion;

        ///<summary>Default style to use for subtitles.</summary>
        property TimedTextStyle^ SubtitleStyle;

        ///<summary>Enable conversion of ANSI encoded subtitles to UTF-8.</summary>
        property bool AutoCorrectAnsiSubtitles;

        ///<summary>The character encoding used to decode ANSI encoded subtitles. By default, the active windows codepage is used.</summary>
        property CharacterEncoding^ AnsiSubtitleEncoding
        {
            void set(CharacterEncoding^ value)
            {
                if (value == nullptr)
                    throw ref new InvalidArgumentException();
                m_CharacterEncoding = value;
            }
            CharacterEncoding^ get()
            {
                return m_CharacterEncoding;
            }
        }

        ///<summary>The subtitle delay will be initially applied to all subtitle tracks.
        ///Use SetSubtitleDelay() on the FFmpegMediaSource instance if you want to change the delay during playback.</summary>
        property TimeSpan DefaultSubtitleDelay;

        /// <summary>FFmpegMediaSource will seek to the closest video keyframe, if set to true.</summary>
        /// <remarks>
        /// For FastSeek to work, you must use the MediaPlayer for playback, and assign
        /// MediaPlayer.PlaybackSession to the FFmpegMediaSource.PlaybackSession property.
        /// </remarks>
        property bool FastSeek;

        ///<summary>Ensure that audio plays without artifacts after fast seeking.</summary>
        ///<remarks>This will slightly reduce the speed of fast seeking. Enabled by default.</remarks>
        property bool FastSeekCleanAudio;

        ///<summary>Try to improve stream switching times when FastSeek is enabled.</summary>
        property bool FastSeekSmartStreamSwitching;

        ///<summary>The default name to use for audio streams.</summary>
        property String^ DefaultAudioStreamName;

        ///<summary>The default name to use for subtitle streams.</summary>
        property String^ DefaultSubtitleStreamName;

        ///<summary>The default name to use for external subtitle streams.</summary>
        property String^ DefaultExternalSubtitleStreamName;

        ///<summary>Use subtitle font files that are embedded in the media file.</summary>
        property bool UseEmbeddedSubtitleFonts;

        ///<summary>The folder where attachments such as fonts are stored (inside the app's temp folder).</summary>
        property String^ AttachmentCacheFolderName;

        ///<summary>The minimum amount of time a subtitle should be shown. Default is 0.</summary>
        property TimeSpan MinimumSubtitleDuration;

        ///<summary>Each subtitle's duration is extended by this amount. Default is 0.</summary>
        property TimeSpan AdditionalSubtitleDuration;

        ///<summary>Try to prevent overlapping subtitles when extending durations.</summary>
        property bool PreventModifiedSubtitleDurationOverlap;


        ///<summary>Initial FFmpeg video filters. Might be changed later through FFmpegMediaSource.SetFFmpegVideoFilters().</summary>
        ///<remarks>Using FFmpeg video filters will degrade playback performance, since they run on the CPU and not on the GPU.</remarks>
        property String^ FFmpegVideoFilters;

        ///<summary>Initial FFmpeg audio filters. Might be changed later through FFmpegMediaSource.SetFFmpegAudioFilters().</summary>
        property String^ FFmpegAudioFilters;

        ///<summary>Downmix multi-channel audio streams to stereo format.</summary>
        property bool DownmixAudioStreamsToStereo;


    internal:
        /*Internal use:determines if a FFmpegInteropInstance is in frame grabber mode. This mode is used to grab frames from a video stream.*/
        property bool IsFrameGrabber;
        /*Internal use:determines if a FFmpegInteropInstance is in external subtitle parser mode. This mode is used to parse files which contain only subtitle streams*/
        property bool IsExternalSubtitleParser;

        /*Used to pass additional, specific options to external sub parsers*/
        property PropertySet^ AdditionalFFmpegSubtitleOptions;



    private:
        CharacterEncoding^ m_CharacterEncoding;

    };

}