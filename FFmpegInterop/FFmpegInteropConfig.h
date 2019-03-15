#pragma once
#include <CharacterEncoding.h>
#include "TimeSpanHelpers.h"

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Media::Core;

namespace FFmpegInterop
{
	public ref class FFmpegInteropConfig sealed
	{
	public:
		FFmpegInteropConfig()
		{
			PassthroughAudioMP3 = false;
			PassthroughAudioAAC = false;

			PassthroughVideoH264 = true;
			PassthroughVideoH264Hi10P = false; // neither Windows codecs nor known HW decoders support Hi10P
			PassthroughVideoHEVC = true;
			PassthroughVideoWMV3 = true;
			PassthroughVideoVC1 = true;
			PassthroughVideoMPEG2 = false; // requires "MPEG-2 Video Extensions"
			PassthroughVideoVP9 = false; // requires "VP9 Video Extensions"

			VideoOutputAllowIyuv = false;
			VideoOutputAllow10bit = false;
			VideoOutputAllowBgra8 = false;
			VideoOutputAllowNv12 = true;

			SkipErrors = 50;
			MaxAudioThreads = 2;

			MaxSupportedPlaybackRate = 4.0;
			StreamBufferSize = 16384;

			FFmpegOptions = ref new PropertySet();

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
			SubtitleStyle->OutlineColor = { 0x80, 0, 0, 0 };

			AutoCorrectAnsiSubtitles = true;
			AnsiSubtitleEncoding = CharacterEncoding::GetSystemDefault();

			DefaultAudioStreamName = "Audio Stream";
			DefaultSubtitleStreamName = "Subtitle";
			DefaultExternalSubtitleStreamName = "External Subtitle";

			AttachmentCacheFolderName = "FFmpegAttachmentCache";
			UseEmbeddedSubtitleFonts = true;
		};

		property bool PassthroughAudioMP3;
		property bool PassthroughAudioAAC;

		property bool PassthroughVideoH264;
		property bool PassthroughVideoH264Hi10P;
		property bool PassthroughVideoHEVC;
		property bool PassthroughVideoWMV3;
		property bool PassthroughVideoVC1;
		property bool PassthroughVideoMPEG2;
		property bool PassthroughVideoVP9;

		property bool VideoOutputAllowIyuv;
		property bool VideoOutputAllow10bit;
		property bool VideoOutputAllowBgra8;
		property bool VideoOutputAllowNv12;

		/*The maximum number of broken frames to skipp in a stream before stopping decoding*/
		property unsigned int SkipErrors;

		property unsigned int MaxVideoThreads;
		property unsigned int MaxAudioThreads;

		/*The maximum supported playback rate. This is set on the media stream source itself. Does not modify what the transport control default UI shows as available playback speeds. Custom UI necessary*/
		property double MaxSupportedPlaybackRate;
		property unsigned int StreamBufferSize;

		property PropertySet^ FFmpegOptions;

		property TimedTextRegion^ SubtitleRegion;
		property TimedTextStyle^ SubtitleStyle;
		property bool AutoSelectForcedSubtitles;
		property bool OverrideSubtitleStyles;
		/*Used to force conversion of ANSII encoded subtitles to Unicode.*/
		property bool AutoCorrectAnsiSubtitles;
		/*The code page used to decode ANSII encoded subtitles. By default, the active windows codepage is used*/
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
		///Use SetSubtitleDelay() on the FFmpegInteropMSS instance if you want to change the delay during playback.</summary>
		property TimeSpan DefaultSubtitleDelay;
		

		property String^ DefaultAudioStreamName;
		property String^ DefaultSubtitleStreamName;
		property String^ DefaultExternalSubtitleStreamName;

		property bool UseEmbeddedSubtitleFonts;
		property String^ AttachmentCacheFolderName;

		/*Used when the duration of a timed metadata cue could not be resolved*/
		property TimeSpan DefaultTimedMetadataCueDuration;
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