#pragma once
#include "pch.h"
#include "MediaSourceConfig.g.h"
using namespace winrt::Windows::Foundation::Collections;
// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

#define PROPERTY( name, type, defaultValue ) \
    public: \
        type name() { return _##name; } \
        void name(type value) { _##name = value; } \
    private: \
        type _##name = defaultValue;

#define PROPERTY_CONST( name, type, defaultValue ) \
    public: \
        type name() { return _##name; } \
        void name(type const& value) { _##name = value; } \
    private: \
        type _##name = defaultValue;

namespace winrt::FFmpegInteropX::implementation
{
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Windows::Foundation::Collections;
    using namespace winrt::Windows::Media::Core;

    struct MediaSourceConfig : MediaSourceConfigT<MediaSourceConfig>
    {
        ///<summary>Enable passthrough for MP3 audio to system decoder.</summary>
        ///<remarks>This could allow hardware decoding on some platforms (e.g. Windows Phone).</remarks>
        PROPERTY(PassthroughAudioMP3, bool, false);

        ///<summary>Enable passthrough for AAC audio to system decoder.</summary>
        ///<remarks>This could allow hardware decoding on some platforms (e.g. Windows Phone).</remarks>
        PROPERTY(PassthroughAudioAAC, bool, false);

        ///<summary>Sets the video decoder mode. Default is Automatic.</summary>
        PROPERTY(VideoDecoderMode, FFmpegInteropX::VideoDecoderMode, VideoDecoderMode::Automatic);

        ///<summary>Sets the HDR color support mode. Default is Automatic.</summary>
        PROPERTY(HdrSupport, FFmpegInteropX::HdrSupport, HdrSupport::Automatic);

        ///<summary>Max profile allowed for H264 system decoder. Default: High Profile (100). See FF_PROFILE_H264_* values.</summary>
        PROPERTY(SystemDecoderH264MaxProfile, int32_t, FF_PROFILE_H264_HIGH);

        ///<summary>Max level allowed for H264 system decoder. Default: Level 4.1 (41). Use -1 to disable level check.</summary>
        ///<remarks>Most H264 HW decoders only support Level 4.1, so this is the default.</remarks>
        PROPERTY(SystemDecoderH264MaxLevel, int32_t, 41);


        ///<summary>Max profile allowed for HEVC system decoder. Default: High10 Profile (2). See FF_PROFILE_HEVC_* values.</summary>
        PROPERTY(SystemDecoderHEVCMaxProfile, int32_t, FF_PROFILE_HEVC_MAIN_10);

        ///<summary>Max level allowed for HEVC system decoder. Default: Disabled (-1).</summary>
        ///<remarks>Encoded as: 30*Major + 3*Minor. So Level 6.0 = 30*6 = 180, 5.1 = 30*5 + 3*1 = 163, 4.1 = 123.
        ///Many HEVC HW decoders support even very high levels, so we disable the check by default.</remarks>
        PROPERTY(SystemDecoderHEVCMaxLevel, int32_t, -1);

        ///<summary>Allow video output in IYuv format.</summary>
        PROPERTY(VideoOutputAllowIyuv, bool, false);

        ///<summary>Allow video output in 10bit formats.</summary>
        PROPERTY(VideoOutputAllow10bit, bool, true);

        ///<summary>Allow video output in BGRA format - required for video transparency.</summary>
        PROPERTY(VideoOutputAllowBgra8, bool, false);

        ///<summary>Allow video output in NV12 format.</summary>
        PROPERTY(VideoOutputAllowNv12, bool, true);

        ///<summary>The maximum number of broken frames or packets to skip in a stream before stopping decoding.</summary>
        PROPERTY(SkipErrors, int32_t, 50);

        ///<summary>The maximum number of video decoding threads. Setting to means using the number of logical CPU cores.</summary>
        PROPERTY(MaxVideoThreads, int32_t, 0);

        ///<summary>The maximum number of audio decoding threads. Setting to means using the number of logical CPU cores.</summary>
        PROPERTY(MaxAudioThreads, int32_t, 2);

        ///<summary>The maximum supported playback rate. This is set on the media stream source itself. 
        /// This does not modify what the transport control default UI shows as available playback speeds. Custom UI is necessary!</summary>
        PROPERTY(MaxSupportedPlaybackRate, double, 4.0);

        ///<summary>The buffer size in bytes to use for Windows.Storage.Streams.IRandomAccessStream sources.</summary>
        //[deprecated("Deprecated due to irritating name. Use ReadAheadBufferSize and ReadAheadBufferDuration instead.", deprecate, 1)]
        PROPERTY(StreamBufferSize, int32_t, 16384);

        ///<summary>The maximum number of bytes to read in one chunk for Windows.Storage.Streams.IRandomAccessStream sources.</summary>
        PROPERTY(FileStreamReadSize, int32_t, 16384);

        ///<summary>Additional options to use when creating the ffmpeg AVFormatContext.</summary>
        PROPERTY_CONST(FFmpegOptions, PropertySet, PropertySet());


        ///<summary>The default BufferTime that gets assigned to the MediaStreamSource for Windows.Storage.Streams.IRandomAccessStream sources.</summary>
        ///<remarks>Deprecated due to framework bugs and memory consumption. Use ReadAheadBufferSize and ReadAheadBufferDuration instead.</remarks>
        PROPERTY_CONST(DefaultBufferTime, TimeSpan, TimeSpan{ 0 });

        ///<summary>The default BufferTime that gets assigned to the MediaStreamSource for URI sources.</summary>
        ///<remarks>Deprecated due to framework bugs and memory consumption. Use ReadAheadBufferSize and ReadAheadBufferDuration instead.</remarks>
        PROPERTY_CONST(DefaultBufferTimeUri, TimeSpan, TimeSpan{ 0 });


        ///<summary>Enables or disables the read-ahead buffer.</summary>
        ///<remarks>This value can be changed any time during playback.</remarks>
        PROPERTY(ReadAheadBufferEnabled, bool, false);

        ///<summary>The maximum number of bytes to buffer ahead per stream.</summary>
        ///<remarks>This value can be changed any time during playback.</remarks>
        PROPERTY(ReadAheadBufferSize, int64_t, 100*1024*1024);

        ///<summary>The maximum duration to buffer ahead per stream.</summary>
        ///<remarks>This value can be changed any time during playback.</remarks>
        PROPERTY_CONST(ReadAheadBufferDuration, TimeSpan, TimeSpan{ 600000000 });


        ///<summary>Automatically select subtitles when they have the 'forced' flag set.</summary>
        PROPERTY(AutoSelectForcedSubtitles, bool, true);

        ///<summary>Use SubtitleRegion and SubtitleStyle from config class, even if custom styles are defined for a subtitle.</summary>
        PROPERTY(OverrideSubtitleStyles, bool, false);

        ///<summary>Default region to use for subtitles.</summary>
        PROPERTY_CONST(SubtitleRegion, TimedTextRegion, CreateDefaultSubtitleRegion());

        ///<summary>Default style to use for subtitles.</summary>
        PROPERTY_CONST(SubtitleStyle, TimedTextStyle, CreateDefaultSubtitleStyle());

        /// <summary>
        /// The character encoding used for external subtitle files.
        ///
        /// When null, auto detection is used.
        /// If ANSI encoding is auto detected, will use ExternalSubtitleAnsiEncoding.
        /// </summary>
        PROPERTY_CONST(ExternalSubtitleEncoding, FFmpegInteropX::CharacterEncoding, nullptr);

        /// <summary>
        /// The character encoding to use if ANSI encoding is detected for external subtitle files.
        /// </summary>
        PROPERTY_CONST(ExternalSubtitleAnsiEncoding, FFmpegInteropX::CharacterEncoding, CharacterEncoding::GetSystemDefault());

        ///<summary>The subtitle delay will be initially applied to all subtitle tracks.
        ///Use SetSubtitleDelay() on the FFmpegMediaSource instance if you want to change the delay during playback.</summary>
        PROPERTY_CONST(DefaultSubtitleDelay, TimeSpan, TimeSpan{ 0 });

        /// <summary>FFmpegMediaSource will seek to the closest video keyframe, if set to true.</summary>
        /// <remarks>
        /// For FastSeek to work, you must use the MediaPlayer for playback, and assign
        /// MediaPlayer.PlaybackSession to the FFmpegMediaSource.PlaybackSession property.
        /// </remarks>
        PROPERTY(FastSeek, bool, true);

        ///<summary>Ensure that audio plays without artifacts after fast seeking.</summary>
        ///<remarks>This will slightly reduce the speed of fast seeking. Enabled by default.</remarks>
        PROPERTY(FastSeekCleanAudio, bool, true);

        ///<summary>Try to improve stream switching times when FastSeek is enabled.</summary>
        PROPERTY(FastSeekSmartStreamSwitching, bool, true);

        ///<summary>The default name to use for audio streams.</summary>
        PROPERTY_CONST(DefaultAudioStreamName, hstring, L"Audio Stream");

        ///<summary>The default name to use for subtitle streams.</summary>
        PROPERTY_CONST(DefaultSubtitleStreamName, hstring, L"Subtitle");

        ///<summary>The default name to use for external subtitle streams.</summary>
        PROPERTY_CONST(DefaultExternalSubtitleStreamName, hstring, L"External Subtitle");

        ///<summary>Use subtitle font files that are embedded in the media file.</summary>
        PROPERTY(UseEmbeddedSubtitleFonts, bool, true);

        ///<summary>The folder where attachments such as fonts are stored (inside the app's temp folder).</summary>
        PROPERTY_CONST(AttachmentCacheFolderName, hstring, L"FFmpegAttachmentCache");

        ///<summary>The minimum amount of time a subtitle should be shown. Default is 0.</summary>
        PROPERTY_CONST(MinimumSubtitleDuration, TimeSpan, TimeSpan{ 0 });

        ///<summary>Each subtitle's duration is extended by this amount. Default is 0.</summary>
        PROPERTY_CONST(AdditionalSubtitleDuration, TimeSpan, TimeSpan{ 0 });

        ///<summary>Try to prevent overlapping subtitles when extending durations.</summary>
        PROPERTY(PreventModifiedSubtitleDurationOverlap, bool, true);

        ///<summary>Initial FFmpeg video filters. Might be changed later through FFmpegMediaSource.SetFFmpegVideoFilters().</summary>
        ///<remarks>Using FFmpeg video filters will degrade playback performance, since they run on the CPU and not on the GPU.</remarks>
        PROPERTY_CONST(FFmpegVideoFilters, hstring, {});

        ///<summary>Initial FFmpeg audio filters. Might be changed later through FFmpegMediaSource.SetFFmpegAudioFilters().</summary>
        PROPERTY_CONST(FFmpegAudioFilters, hstring, {});

        ///<summary>Downmix multi-channel audio streams to stereo format.</summary>
        PROPERTY(DownmixAudioStreamsToStereo, bool, false);

        ///<summary>Automatically extend the duration of the MediaStreamSource, if the file unexpectedly contains additional data.</summary>
        PROPERTY(AutoExtendDuration, bool, true);

        ///<summary>Keep metadata available after MediaSource was closed.</summary>
        ///<remarks>Set this to false to cleanup more memory automatically, if you are sure you don't need metadata after playback end.</remarks>
        PROPERTY(KeepMetadataOnMediaSourceClosed, bool, true);

    public:
        //internal:
        bool IsFrameGrabber;
        /*Internal use:determines if a FFmpegInteropInstance is in external subtitle parser mode. This mode is used to parse files which contain only subtitle streams*/
        bool IsExternalSubtitleParser;
        //Apply hdr color info, if available in the file
        bool ApplyHdrColorInfo;

        /*Used to pass additional, specific options to external sub parsers*/
        PropertySet AdditionalFFmpegSubtitleOptions = {nullptr};

    private:

        TimedTextRegion CreateDefaultSubtitleRegion()
        {
            auto region = TimedTextRegion();
            TimedTextSize extent;
            extent.Unit = TimedTextUnit::Percentage;
            extent.Width = 100;
            extent.Height = 88;
            region.Extent(extent);
            TimedTextPoint position;
            position.Unit = TimedTextUnit::Pixels;
            position.X = 0;
            position.Y = 0;
            region.Position(position);
            region.DisplayAlignment(TimedTextDisplayAlignment::After);
            region.Background(winrt::Windows::UI::Colors::Transparent());
            region.ScrollMode(TimedTextScrollMode::Rollup);
            region.TextWrapping(TimedTextWrapping::Wrap);
            region.WritingMode(TimedTextWritingMode::LeftRightTopBottom);
            region.IsOverflowClipped(false);
            region.ZIndex(0);
            TimedTextDouble LineHeight;
            LineHeight.Unit = TimedTextUnit::Percentage;
            LineHeight.Value = 100;
            region.LineHeight(LineHeight);
            TimedTextPadding padding;
            padding.Unit = TimedTextUnit::Percentage;
            padding.Start = 0;
            padding.After = 0;
            padding.Before = 0;
            padding.End = 0;
            region.Padding(padding);
            region.Name(L"");
            return region;
        }

        TimedTextStyle CreateDefaultSubtitleStyle()
        {
            auto style = TimedTextStyle();
            style.FontFamily(L"default");
            TimedTextDouble fontSize;
            fontSize.Unit = TimedTextUnit::Percentage;
            fontSize.Value = 100;
            style.FontSize(fontSize);
            style.LineAlignment(TimedTextLineAlignment::Center);
            style.FontStyle(TimedTextFontStyle::Normal);
            style.FontWeight(TimedTextWeight::Normal);
            style.Foreground(winrt::Windows::UI::Colors::White());
            style.Background(Windows::UI::Colors::Transparent());
            TimedTextDouble outlineThickness;
            outlineThickness.Unit = TimedTextUnit::Percentage;
            outlineThickness.Value = 4.5;
            style.OutlineThickness(outlineThickness);
            style.FlowDirection(TimedTextFlowDirection::LeftToRight);
            style.OutlineColor(winrt::Windows::UI::Color{ 0x80, 0, 0, 0 });
            return style;
        }
    };
}
namespace winrt::FFmpegInteropX::factory_implementation
{
    struct MediaSourceConfig : MediaSourceConfigT<MediaSourceConfig, implementation::MediaSourceConfig>
    {
    };
}
