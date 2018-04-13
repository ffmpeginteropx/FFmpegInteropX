#pragma once

#include "SubtitleProvider.h"

#include <string>
#include <codecvt>

namespace FFmpegInterop
{
	ref class SubtitleProviderSrt : SubtitleProvider
	{
	internal:
		SubtitleProviderSrt(FFmpegReader^ reader,
			AVFormatContext* avFormatCtx,
			AVCodecContext* avCodecCtx,
			FFmpegInteropConfig^ config,
			int index)
			: SubtitleProvider(reader, avFormatCtx, avCodecCtx, config, index, TimedMetadataKind::Subtitle)
		{
		}

		virtual IMediaCue^ CreateCue(AVPacket* packet) override
		{
			auto str = utf8_to_wstring(std::string((char*)packet->data, packet->size));

			// TODO we could try to forward some font style tags (if whole text is wrapped in <i> or <b>)
			// TODO we might also have to look for &nbsp; and others?

			// strip html tags from string
			while (true)
			{
				auto nextEffect = str.find('<');
				if (nextEffect >= 0)
				{
					auto endEffect = str.find('>', nextEffect);
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

			auto timedText = convertFromString(str);

			TimedTextCue^ cue = ref new TimedTextCue();
			cue->CueRegion = m_config->SubtitleRegion;
			cue->CueStyle = m_config->SubtitleStyle;

			TimedTextLine^ textLine = ref new TimedTextLine();
			textLine->Text = timedText;
			cue->Lines->Append(textLine);

			return cue;
		}

	};
}