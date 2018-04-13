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
			int index)
			: SubtitleProvider(reader, avFormatCtx, avCodecCtx, config, index, TimedMetadataKind::Subtitle)
		{
		}

		// convert UTF-8 string to wstring
		std::wstring utf8_to_wstring(const std::string& str)
		{
			std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
			return myconv.from_bytes(str);
		}

		Platform::String ^ convertFromString(const std::wstring & input)
		{
			return ref new Platform::String(input.c_str(), (unsigned int)input.length());
		}

		virtual IMediaCue^ CreateCue(AVPacket* packet) override
		{
			auto str = utf8_to_wstring(std::string((char*)packet->data, packet->size));

			// TODO we could parse style definitions from extradata and forward bold and italic

			// strip effects from string
			while (true)
			{
				auto nextEffect = str.find('{');
				if (nextEffect >= 0)
				{
					auto endEffect = str.find('}', nextEffect);
					if (endEffect > nextEffect)
					{
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

			// now text should be last element of comma separated list
			auto lastComma = str.find_last_of(',');
			if (lastComma > 0 && lastComma < str.length() - 1)
			{
				str = str.substr(lastComma + 1);
				auto timedText = convertFromString(str);

				TimedTextCue^ cue = ref new TimedTextCue();
				cue->CueRegion = m_config->SubtitleRegion;
				cue->CueStyle = m_config->SubtitleStyle;

				TimedTextLine^ textLine = ref new TimedTextLine();
				textLine->Text = timedText;
				cue->Lines->Append(textLine);

				return cue;
			}

			return nullptr;
		}

	};
}