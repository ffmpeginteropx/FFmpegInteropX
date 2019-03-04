#pragma once

#include "SubtitleProvider.h"

#include <string>
#include <codecvt>

namespace FFmpegInterop
{
	ref class SubtitleProviderSsaAss : SubtitleProvider
	{
	internal:
		SubtitleProviderSsaAss(FFmpegReader^ reader,
			AVFormatContext* avFormatCtx,
			AVCodecContext* avCodecCtx,
			FFmpegInteropConfig^ config,
			int index,
			CoreDispatcher^ dispatcher)
			: SubtitleProvider(reader, avFormatCtx, avCodecCtx, config, index, TimedMetadataKind::Subtitle, dispatcher)
		{
		}

		virtual HRESULT Initialize() override
		{
			auto hr = SubtitleProvider::Initialize();
			if (SUCCEEDED(hr))
			{
				ssaVersion = 4;
				if (m_pAvCodecCtx->subtitle_header && m_pAvCodecCtx->subtitle_header_size > 0)
				{
					auto str = std::string((char*)m_pAvCodecCtx->subtitle_header, m_pAvCodecCtx->subtitle_header_size);
					auto versionIndex = str.find("ScriptType: v4.0.0+");
					if (versionIndex != str.npos)
					{
						isAss = true;
					}
					else
					{
						versionIndex = str.find("ScriptType: v");
						if (versionIndex != str.npos && versionIndex + 13 < str.length())
						{
							auto version = str.at(versionIndex + 13) - '0';
							if (version > 0 && version < 9)
							{
								ssaVersion = version;
							}
						}
					}
					
					auto resx = str.find("\nPlayResX: ");
					auto resy = str.find("\nPlayResY: ");
					if (resx != str.npos && resy != str.npos)
					{
						int w, h;
						if (sscanf_s((char*)m_pAvCodecCtx->subtitle_header + resx, "\nPlayResX: %i\n", &w) == 1 &&
							sscanf_s((char*)m_pAvCodecCtx->subtitle_header + resy, "\nPlayResY: %i\n", &h) == 1)
						{
							width = w;
							height = h;
						}
					}

					if (isAss)
					{
						ReadStylesV4Plus(str);
					}
					else if (ssaVersion == 4)
					{
						ReadStylesV4(str);
					}
				}

				if (ssaVersion >= 3)
				{
					textIndex = 9;
				}
				else
				{
					textIndex = 8;
				}
			}

			return hr;
		}

		virtual IMediaCue^ CreateCue(AVPacket* packet, TimeSpan* position, TimeSpan *duration) override
		{
			AVSubtitle subtitle;
			int gotSubtitle = 0;
			auto result = avcodec_decode_subtitle2(m_pAvCodecCtx, &subtitle, &gotSubtitle, packet);
			if (result > 0 && gotSubtitle && subtitle.num_rects > 0)
			{
				auto str = utf8_to_wstring(std::string(subtitle.rects[0]->ass));
				
				int startStyle = -1;
				int endStyle = -1;
				int lastComma = -1;
				bool hasError = false;
				for (int i = 0; i < textIndex; i++)
				{
					auto nextComma = str.find(',', lastComma + 1);
					if (nextComma != str.npos)
					{
						if (i == styleIndex)
						{
							startStyle = (int)nextComma + 1;
						}
						else if (i == styleIndex + 1)
						{
							endStyle = (int)nextComma;
						}
						lastComma = (int)nextComma;
					}
					else
					{
						// this should not happen. still we try to be graceful. let's use what we found.
						hasError = true;
						break;
					}
				}

				SsaStyleDefinition^ style = nullptr;
				if (!hasError && startStyle > 0 && endStyle > 0)
				{
					auto styleName = convertFromString(str.substr(startStyle, endStyle - startStyle));
					if (styles.find(styleName) != styles.end())
					{
						style = styles[styleName];
					}
				}

				if (lastComma > 0 && lastComma < (int)str.length() - 1)
				{
					// get actual text
					str = str.substr(lastComma + 1);

					find_and_replace(str, L"\\N", L"\n");
					find_and_replace(str, L"\\h", L"\t");
					str.erase(str.find_last_not_of(L" \n\r") + 1);

					auto cueStyle = !m_config->OverrideSubtitleStyles && style ? style->Style : m_config->SubtitleStyle;
					auto cueRegion = !m_config->OverrideSubtitleStyles && style ? style->Region : m_config->SubtitleRegion;

					TimedTextCue^ cue = ref new TimedTextCue();
					cue->CueRegion = cueRegion;
					cue->CueStyle = cueStyle;

					TimedTextLine^ textLine = ref new TimedTextLine();
					cue->Lines->Append(textLine);

					TimedTextStyle^ subStyle = nullptr;
					TimedTextSubformat^ subFormat = nullptr;
					while (true)
					{
						auto nextEffect = str.find('{');
						if (nextEffect != str.npos)
						{
							auto endEffect = str.find('}', nextEffect);
							if (endEffect != str.npos)
							{
								auto effect = str.substr(nextEffect, endEffect - nextEffect + 1);

								// create a subformat with default style for beginning of text (UWP seems to need subformats for all text, if used)
								if (nextEffect > 0 && subFormat == nullptr)
								{
									subFormat = ref new TimedTextSubformat();
									subFormat->SubformatStyle = cueStyle;
									subFormat->StartIndex = 0;
								}
							
								// apply previous subformat, if any
								if (subFormat != nullptr)
								{
									subFormat->Length = nextEffect - subFormat->StartIndex;
									textLine->Subformats->Append(subFormat);
								}

								// create new subformat for following text
								subStyle = subStyle != nullptr ? CopyStyle(subStyle) : CopyStyle(cueStyle);
								subFormat = ref new TimedTextSubformat();
								subFormat->SubformatStyle = subStyle;
								subFormat->StartIndex = nextEffect;

								auto fnIndex = effect.find(L"\\fn");
								if (fnIndex != effect.npos)
								{
									//{\fnArial} or {\fnArial\different effect}
									auto fnName = effect.substr(fnIndex + 3);
									auto bracIndex = fnName.find(L"}");
									if (bracIndex != fnName.npos)
									{
										fnName = fnName.substr(0, bracIndex);
										subStyle->FontFamily = convertFromString(fnName);
									}
									auto backIndex = fnName.find(L"\\");
									if (backIndex != fnName.npos)
									{
										fnName = fnName.substr(0, backIndex);
										subStyle->FontFamily = convertFromString(fnName);
									}
								}
								auto fsIndex = effect.find(L"\\fs");
								if (fsIndex != effect.npos)
								{
									auto fSize = effect.substr(fsIndex + 3);
									auto bracIndex = fSize.find(L"}");
									if (bracIndex != fSize.npos)
									{
										fSize = fSize.substr(0, bracIndex);
										TimedTextDouble fontSize;
										fontSize.Unit = TimedTextUnit::Pixels;
										fontSize.Value = convertToDouble(convertFromString(fSize));
										subStyle->FontSize = fontSize;
									}
									auto backIndex = fSize.find(L"\\");
									if (backIndex != fSize.npos)
									{
										fSize = fSize.substr(0, backIndex);
										TimedTextDouble fontSize;
										fontSize.Unit = TimedTextUnit::Pixels;
										fontSize.Value = convertToDouble(convertFromString(fSize));
										subStyle->FontSize = fontSize;
									}
								}
								// \c&H<bb><gg><rr>&			primary fill color
								// \1c&H<bb><gg><rr>&			primary fill color
								// \2c&H<bb><gg><rr>&			secondary fill color
								// \3c&H<bb><gg><rr>&			border color
								// \4c&H<bb><gg><rr>&			shadow color
								auto fcIndex = effect.find(L"\\c"); // \c and \1c are same and effect to primary color
								auto fc1Index = effect.find(L"\\1c");
								if (fcIndex != effect.npos || fc1Index != effect.npos)
								{
									//\c&HDD19C9&		color=> purple
									auto fColor = effect;
									if (fcIndex != effect.npos)// \c
										fColor = fColor.substr(fcIndex + 2);

									if (fc1Index != effect.npos)// \1c
										fColor = fColor.substr(fc1Index + 3);
									auto bracIndex = fColor.find(L"}");
									if (bracIndex != fColor.npos)
									{
										fColor = fColor.substr(0, bracIndex);
										find_and_replace(fColor, L"&", L"");
										find_and_replace(fColor, L"H", L"");
										
										int color = convertHexToInt(convertFromString(fColor));
										subStyle->Foreground = ColorFromArgb(color << 8 | 0x000000FF);
									}
									auto backIndex = fColor.find(L"\\");
									if (backIndex != fColor.npos)
									{
										fColor = fColor.substr(0, backIndex);
										find_and_replace(fColor, L"&", L"");
										find_and_replace(fColor, L"H", L""); 
										
										int color = convertHexToInt(convertFromString(fColor));
										subStyle->Foreground = ColorFromArgb(color << 8 | 0x000000FF);
									}
								}
								auto fsecIndex = effect.find(L"\\3c");
								if (fsecIndex != effect.npos)
								{
									auto fColor = effect.substr(fsecIndex + 3);
									auto bracIndex = fColor.find(L"}");
									if (bracIndex != fColor.npos)
									{
										fColor = fColor.substr(0, bracIndex);
										find_and_replace(fColor, L"&", L"");
										find_and_replace(fColor, L"H", L"");

										int color = convertHexToInt(convertFromString(fColor));
										subStyle->OutlineColor = ColorFromArgb(color << 8 | 0x000000FF);
									}
									auto backIndex = fColor.find(L"\\");
									if (backIndex != fColor.npos)
									{
										fColor = fColor.substr(0, backIndex);
										find_and_replace(fColor, L"&", L"");
										find_and_replace(fColor, L"H", L"");

										int color = convertHexToInt(convertFromString(fColor));
										subStyle->OutlineColor = ColorFromArgb(color << 8 | 0x000000FF);
									}
								}
								if (effect.find(L"\\b1") != effect.npos)
								{
									subStyle->FontWeight = TimedTextWeight::Bold;
								}
								else if (effect.find(L"\\b0") != effect.npos)
								{
									subStyle->FontWeight = TimedTextWeight::Normal;
								}

								if (Windows::Foundation::Metadata::ApiInformation::IsPropertyPresent("Windows.Media.Core.TimedTextStyle", "FontStyle"))
								{
									if (effect.find(L"\\i1") != effect.npos)
									{
										subStyle->FontStyle = TimedTextFontStyle::Italic;
									}
									else if (effect.find(L"\\i0") != effect.npos)
									{
										subStyle->FontStyle = TimedTextFontStyle::Normal;
									}
								}

								if (Windows::Foundation::Metadata::ApiInformation::IsPropertyPresent("Windows.Media.Core.TimedTextStyle", "IsUnderlineEnabled"))
								{
									if (effect.find(L"\\u1") != effect.npos)
									{
										subStyle->IsUnderlineEnabled = true;
									}
									else if (effect.find(L"\\u0") != effect.npos)
									{
										subStyle->IsUnderlineEnabled = false;
									}
								}

								if (Windows::Foundation::Metadata::ApiInformation::IsPropertyPresent("Windows.Media.Core.TimedTextStyle", "IsLineThroughEnabled"))
								{
									if (effect.find(L"\\s1") != effect.npos)
									{
										subStyle->IsLineThroughEnabled = true;
									}
									else if (effect.find(L"\\s0") != effect.npos)
									{
										subStyle->IsLineThroughEnabled = false;
									}
								}

								// strip effect from actual text
								if (endEffect < str.length() - 1)
								{
									str = str.substr(0, nextEffect).append(str.substr(endEffect + 1));
								}
								else
								{
									str = str.substr(0, nextEffect);
								}
							}
							else
							{
								break;
							}
						}
						else
						{
							break;
						}
					}

					// apply last subformat, if any
					if (subFormat != nullptr)
					{
						subFormat->Length = str.size() - subFormat->StartIndex;
						textLine->Subformats->Append(subFormat);
					}
				
					auto timedText = convertFromString(str);
					if (timedText->Length() > 0)
					{
						textLine->Text = timedText;

						return cue;
					}
				}
			}
			else if (result <= 0)
			{
				OutputDebugString(L"Failed to decode subtitle.");
			}

			return nullptr;
		}

		void ReadStylesV4Plus(std::string str)
		{
			auto stylesV4plus = str.find("[V4+ Styles]");
			while (stylesV4plus != str.npos)
			{
				stylesV4plus = str.find("\nStyle: ", stylesV4plus);
				if (stylesV4plus != str.npos)
				{
					stylesV4plus += 8;
					/*
					[V4+ Styles]
					Format: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding
					*/
					const unsigned int MAX_STYLE_NAME_CHARS = 256;
					char name[MAX_STYLE_NAME_CHARS];
					char font[MAX_STYLE_NAME_CHARS];
					int size, color, secondaryColor, outlineColor, backColor;
					int bold, italic, underline, strikeout;
					int scaleX, scaleY, spacing, angle, borderStyle;
					int outline, shadow, alignment;
					int marginL, marginR, marginV, encoding;

					auto count = sscanf_s((char*)m_pAvCodecCtx->subtitle_header + stylesV4plus,
						"%[^,],%[^,],%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i",
						name, MAX_STYLE_NAME_CHARS, font, MAX_STYLE_NAME_CHARS,
						&size, &color, &secondaryColor, &outlineColor, &backColor,
						&bold, &italic, &underline, &strikeout,
						&scaleX, &scaleY, &spacing, &angle, &borderStyle,
						&outline, &shadow, &alignment,
						&marginL, &marginR, &marginV, &encoding);

					if (count == 3)
					{
						// try with hex colors
						count = sscanf_s((char*)m_pAvCodecCtx->subtitle_header + stylesV4plus,
							"%[^,],%[^,],%i,&H%x,&H%x,&H%x,&H%x,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i",
							name, MAX_STYLE_NAME_CHARS, font, MAX_STYLE_NAME_CHARS,
							&size, &color, &secondaryColor, &outlineColor, &backColor,
							&bold, &italic, &underline, &strikeout,
							&scaleX, &scaleY, &spacing, &angle, &borderStyle,
							&outline, &shadow, &alignment,
							&marginL, &marginR, &marginV, &encoding);
					}

					if (count == 23)
					{
						auto verticalAlignment =
							alignment <= 3 ? TimedTextDisplayAlignment::After :
							alignment <= 6 ? TimedTextDisplayAlignment::Center :
							TimedTextDisplayAlignment::Before;

						auto horizontalAlignment =
							alignment == 2 || alignment == 5 || alignment == 8 ? TimedTextLineAlignment::Center :
							alignment == 1 || alignment == 4 || alignment == 7 ? TimedTextLineAlignment::Start :
							TimedTextLineAlignment::End;

						auto SubtitleRegion = ref new TimedTextRegion();

						TimedTextSize extent;
						extent.Unit = TimedTextUnit::Percentage;
						extent.Width = 100;
						extent.Height = 100;
						SubtitleRegion->Extent = extent;
						TimedTextPoint position;
						position.Unit = TimedTextUnit::Pixels;
						position.X = 0;
						position.Y = 0;
						SubtitleRegion->Position = position;
						SubtitleRegion->DisplayAlignment = verticalAlignment;
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
						if (width > 0 && height > 0)
						{
							padding.Start = (double)marginL * 100 / width;
							padding.End = (double)marginR * 100 / width;
							padding.After = (double)marginV * 100 / height;
						}
						else
						{
							padding.After = 12;
						}
						SubtitleRegion->Padding = padding;
						SubtitleRegion->Name = "";

						auto SubtitleStyle = ref new TimedTextStyle();

						SubtitleStyle->FontFamily = ConvertString(font);
						TimedTextDouble fontSize;
						fontSize.Unit = TimedTextUnit::Pixels;
						fontSize.Value = size;
						SubtitleStyle->FontSize = fontSize;
						SubtitleStyle->LineAlignment = horizontalAlignment;
						if (Windows::Foundation::Metadata::ApiInformation::IsPropertyPresent("Windows.Media.Core.TimedTextStyle", "FontStyle"))
						{
							SubtitleStyle->FontStyle = italic ? TimedTextFontStyle::Italic : TimedTextFontStyle::Normal;
						}
						SubtitleStyle->FontWeight = bold ? TimedTextWeight::Bold : TimedTextWeight::Normal;
						SubtitleStyle->Foreground = ColorFromArgb(color << 8 | 0x000000FF);
						SubtitleStyle->Background = Windows::UI::Colors::Transparent; //ColorFromArgb(backColor);
						TimedTextDouble outlineRadius;
						outlineRadius.Unit = TimedTextUnit::Percentage;
						outlineRadius.Value = outline;
						SubtitleStyle->OutlineRadius = outlineRadius;
						TimedTextDouble outlineThickness;
						outlineThickness.Unit = TimedTextUnit::Percentage;
						outlineThickness.Value = outline;
						SubtitleStyle->OutlineThickness = outlineThickness;
						SubtitleStyle->FlowDirection = TimedTextFlowDirection::LeftToRight;
						SubtitleStyle->OutlineColor = ColorFromArgb(outlineColor << 8 | 0x000000FF);

						if (Windows::Foundation::Metadata::ApiInformation::IsPropertyPresent("Windows.Media.Core.TimedTextStyle", "IsUnderlineEnabled"))
						{
							SubtitleStyle->IsUnderlineEnabled = underline;
						}

						if (Windows::Foundation::Metadata::ApiInformation::IsPropertyPresent("Windows.Media.Core.TimedTextStyle", "IsLineThroughEnabled"))
						{
							SubtitleStyle->IsLineThroughEnabled = strikeout;
						}

						auto style = ref new SsaStyleDefinition();
						style->Name = ConvertString(name);
						style->Region = SubtitleRegion;
						style->Style = SubtitleStyle;

						styles[style->Name] = style;
					}
				}
				else
				{
					break;
				}
			}
		}

		void ReadStylesV4(std::string str)
		{
			auto stylesV4plus = str.find("[V4 Styles]");
			while (stylesV4plus != str.npos)
			{
				stylesV4plus = str.find("\nStyle: ", stylesV4plus);
				if (stylesV4plus != str.npos)
				{
					stylesV4plus += 8;
					/*
					[V4+ Styles]
					Format: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding
					[V4 Styles]
					Format: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, TertiaryColour, BackColour, Bold, Italic, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, AlphaLevel, Encoding
					*/
					const unsigned int MAX_STYLE_NAME_CHARS = 256;
					char name[MAX_STYLE_NAME_CHARS];
					char font[MAX_STYLE_NAME_CHARS];
					int size, color, secondaryColor, outlineColor, backColor;
					int bold, italic, borderstyle;
					int outline, shadow, alignment;
					int marginL, marginR, marginV, alpha, encoding;

					auto count = sscanf_s((char*)m_pAvCodecCtx->subtitle_header + stylesV4plus,
						"%[^,],%[^,],%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i",
						name, MAX_STYLE_NAME_CHARS, font, MAX_STYLE_NAME_CHARS,
						&size, &color, &secondaryColor, &outlineColor, &backColor,
						&bold, &italic, &borderstyle,
						&outline, &shadow, &alignment,
						&marginL, &marginR, &marginV, &alpha, &encoding);

					if (count == 3)
					{
						// try with hex colors
						count = sscanf_s((char*)m_pAvCodecCtx->subtitle_header + stylesV4plus,
							"%[^,],%[^,],%i,&H%x,&H%x,&H%x,&H%x,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i",
							name, MAX_STYLE_NAME_CHARS, font, MAX_STYLE_NAME_CHARS,
							&size, &color, &secondaryColor, &outlineColor, &backColor,
							&bold, &italic, &borderstyle,
							&outline, &shadow, &alignment,
							&marginL, &marginR, &marginV, &alpha, &encoding);
					}

					if (count == 18)
					{
						auto verticalAlignment =
							alignment <= 3 ? TimedTextDisplayAlignment::After :
							alignment <= 7 ? TimedTextDisplayAlignment::Center :
							TimedTextDisplayAlignment::Before;

						auto horizontalAlignment =
							alignment == 2 || alignment == 6 || alignment == 10 ? TimedTextLineAlignment::Center :
							alignment == 1 || alignment == 5 || alignment == 9 ? TimedTextLineAlignment::Start :
							TimedTextLineAlignment::End;

						auto SubtitleRegion = ref new TimedTextRegion();

						TimedTextSize extent;
						extent.Unit = TimedTextUnit::Percentage;
						extent.Width = 100;
						extent.Height = 100;
						SubtitleRegion->Extent = extent;
						TimedTextPoint position;
						position.Unit = TimedTextUnit::Pixels;
						position.X = 0;
						position.Y = 0;
						SubtitleRegion->Position = position;
						SubtitleRegion->DisplayAlignment = verticalAlignment;
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
						if (width > 0 && height > 0)
						{
							padding.Start = (double)marginL * 100 / width;
							padding.End = (double)marginR * 100 / width;
							padding.After = (double)marginV * 100 / height;
						}
						else
						{
							padding.After = 12;
						}
						SubtitleRegion->Padding = padding;
						SubtitleRegion->Name = "";

						auto SubtitleStyle = ref new TimedTextStyle();

						SubtitleStyle->FontFamily = ConvertString(font);
						TimedTextDouble fontSize;
						fontSize.Unit = TimedTextUnit::Pixels;
						fontSize.Value = size;
						SubtitleStyle->FontSize = fontSize;
						SubtitleStyle->LineAlignment = horizontalAlignment;
						if (Windows::Foundation::Metadata::ApiInformation::IsPropertyPresent("Windows.Media.Core.TimedTextStyle", "FontStyle"))
						{
							SubtitleStyle->FontStyle = italic ? TimedTextFontStyle::Italic : TimedTextFontStyle::Normal;
						}
						SubtitleStyle->FontWeight = bold ? TimedTextWeight::Bold : TimedTextWeight::Normal;
						SubtitleStyle->Foreground = ColorFromArgb(color << 8 | 0x000000FF);
						SubtitleStyle->Background = Windows::UI::Colors::Transparent; //ColorFromArgb(backColor);
						TimedTextDouble outlineRadius;
						outlineRadius.Unit = TimedTextUnit::Percentage;
						outlineRadius.Value = outline;
						SubtitleStyle->OutlineRadius = outlineRadius;
						TimedTextDouble outlineThickness;
						outlineThickness.Unit = TimedTextUnit::Percentage;
						outlineThickness.Value = outline;
						SubtitleStyle->OutlineThickness = outlineThickness;
						SubtitleStyle->FlowDirection = TimedTextFlowDirection::LeftToRight;
						SubtitleStyle->OutlineColor = ColorFromArgb(outlineColor << 8 | 0x000000FF);

						auto style = ref new SsaStyleDefinition();
						style->Name = ConvertString(name);
						style->Region = SubtitleRegion;
						style->Style = SubtitleStyle;

						styles[style->Name] = style;
					}
				}
				else
				{
					break;
				}
			}
		}

		TimedTextStyle^ CopyStyle(TimedTextStyle^ style)
		{
			auto copy = ref new TimedTextStyle();
			copy->Background = style->Background;
			copy->FlowDirection = style->FlowDirection;
			copy->FontFamily = style->FontFamily;
			copy->FontSize = style->FontSize;
			copy->FontWeight = style->FontWeight;
			copy->Foreground = style->Foreground;
			copy->OutlineColor = style->OutlineColor;
			copy->OutlineRadius = style->OutlineRadius;
			copy->OutlineThickness = style->OutlineThickness;

			if (Windows::Foundation::Metadata::ApiInformation::IsPropertyPresent("Windows.Media.Core.TimedTextStyle", "FontStyle"))
			{
				copy->FontStyle = style->FontStyle;
			}
			if (Windows::Foundation::Metadata::ApiInformation::IsPropertyPresent("Windows.Media.Core.TimedTextStyle", "IsLineThroughEnabled"))
			{
				copy->IsLineThroughEnabled = style->IsLineThroughEnabled;
			}
			if (Windows::Foundation::Metadata::ApiInformation::IsPropertyPresent("Windows.Media.Core.TimedTextStyle", "IsUnderlineEnabled"))
			{
				copy->IsUnderlineEnabled = style->IsUnderlineEnabled;
			}

			return copy;
		}

		TimedTextRegion^ CopyRegion(TimedTextRegion^ region)
		{
			auto copy = ref new TimedTextRegion();
			copy->Background = region->Background;
			copy->DisplayAlignment = region->DisplayAlignment;
			copy->Extent = region->Extent;
			copy->IsOverflowClipped = region->IsOverflowClipped;
			copy->LineHeight = region->LineHeight;
			copy->Padding = region->Padding;
			copy->Position = region->Position;
			copy->ScrollMode = region->ScrollMode;
			copy->TextWrapping = region->TextWrapping;
			copy->WritingMode = region->WritingMode;
			copy->ZIndex = region->ZIndex;
			return copy;
		}

		Windows::UI::Color ColorFromArgb(int argb)
		{
			auto result = *reinterpret_cast<Windows::UI::Color*>(&argb);
			return result;
		}
		void find_and_replace(std::wstring& source, std::wstring const& find, std::wstring const& replace)
		{
			for (std::wstring::size_type i = 0; (i = source.find(find, i)) != std::wstring::npos;)
			{
				source.replace(i, find.length(), replace);
				i += replace.length();
			}
		}

		ref class SsaStyleDefinition
		{
		public:
			property String^ Name;
			property TimedTextRegion^ Region;
			property TimedTextStyle^ Style;
		};

	private:
		bool isAss;
		int ssaVersion;
		int textIndex;
		int width;
		int height;
		const int styleIndex = 2;
		std::map<String^, SsaStyleDefinition^> styles;
	};
}