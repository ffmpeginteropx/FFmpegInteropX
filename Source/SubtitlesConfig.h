#pragma once
#include "pch.h"
#include "SubtitlesConfig.g.h"
#include "ConfigurationCommon.h"

namespace winrt::FFmpegInteropX::implementation
{
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Windows::Foundation::Collections;
    using namespace winrt::Windows::Media::Core;

    struct SubtitlesConfig : SubtitlesConfigT<SubtitlesConfig>
    {
        SubtitlesConfig() = default;

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
        /// When null, auto detection is used. This is the default and recommended.
        /// If ANSI encoding is auto detected, will use ExternalSubtitleAnsiEncoding.
        /// </summary>
        PROPERTY_CONST(ExternalSubtitleEncoding, FFmpegInteropX::CharacterEncoding, nullptr);

        /// <summary>
        /// The character encoding to use if ANSI encoding is detected for external subtitle files.
        /// </summary>
        PROPERTY_CONST(ExternalSubtitleAnsiEncoding, FFmpegInteropX::CharacterEncoding, CharacterEncoding::SystemLocale());

        ///<summary>The subtitle delay will be initially applied to all subtitle tracks.
        ///Use SetSubtitleDelay() on the FFmpegMediaSource instance if you want to change the delay during playback.</summary>
        PROPERTY_CONST(DefaultSubtitleDelay, TimeSpan, TimeSpan{ 0 });

        ///<summary>The default name to use for subtitle streams.</summary>
        PROPERTY_CONST(DefaultStreamName, hstring, L"Subtitle");

        ///<summary>The default name to use for external subtitle streams.</summary>
        PROPERTY_CONST(DefaultExternalSubtitleStreamName, hstring, L"External Subtitle");

        ///<summary>Use subtitle font files that are embedded in the media file.</summary>
        PROPERTY(UseEmbeddedSubtitleFonts, bool, true);

        ///<summary>The minimum amount of time a subtitle should be shown. Default is 0.</summary>
        PROPERTY_CONST(MinimumSubtitleDuration, TimeSpan, TimeSpan{ 0 });

        ///<summary>Each subtitle's duration is extended by this amount. Default is 0.</summary>
        PROPERTY_CONST(AdditionalSubtitleDuration, TimeSpan, TimeSpan{ 0 });

        ///<summary>Try to prevent overlapping subtitles when extending durations.</summary>
        PROPERTY(PreventModifiedSubtitleDurationOverlap, bool, true);

        ///<summary>Optional gap to keep between cues when extending durations.</summary>
        PROPERTY(ModifiedSubtitleDurationGap, TimeSpan, TimeSpan{ 0 });

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
