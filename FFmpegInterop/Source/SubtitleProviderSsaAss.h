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

		virtual HRESULT Initialize() override
		{
			auto hr = SubtitleProvider::Initialize();
			if (SUCCEEDED(hr))
			{
				ssaVersion = 4;
				if (m_pAvCodecCtx->extradata && m_pAvCodecCtx->extradata_size > 0)
				{
					auto str = std::string((char*)m_pAvCodecCtx->extradata);
					auto versionIndex = str.find("ScriptType: v");
					if (versionIndex >= 0 && versionIndex + 13 < str.length())
					{
						auto version = str.at(versionIndex + 13) - '0';
						if (version > 0 && version < 9)
						{
							ssaVersion = version;
						}
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
				
				int lastComma = -1;
				for (int i = 0; i < textIndex; i++)
				{
					auto nextComma = str.find(',', lastComma + 1);
					if (nextComma >= 0)
					{
						lastComma = nextComma;
					}
					else
					{
						// this should not happen. still we try to be graceful. let's use what we found.
						break;
					}
				}

				if (lastComma > 0 && lastComma < (int)str.length() - 1)
				{
					// get actual text
					str = str.substr(lastComma + 1);

					find_and_replace(str, L"\\N", L"\n");
					str.erase(str.find_last_not_of(L" \n\r") + 1);

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

					// TODO we could parse style definitions from extradata and/or effect and use at least bold and italic
					
					auto timedText = convertFromString(str);

					TimedTextCue^ cue = ref new TimedTextCue();
					cue->CueRegion = m_config->SubtitleRegion;
					cue->CueStyle = m_config->SubtitleStyle;

					TimedTextLine^ textLine = ref new TimedTextLine();
					textLine->Text = timedText;
					cue->Lines->Append(textLine);

					return cue;
				}
			}

			return nullptr;
		}

		void find_and_replace(std::wstring& source, std::wstring const& find, std::wstring const& replace)
		{
			for (std::wstring::size_type i = 0; (i = source.find(find, i)) != std::wstring::npos;)
			{
				source.replace(i, find.length(), replace);
				i += replace.length();
			}
		}


	private:
		int ssaVersion;
		int textIndex;
	};
}