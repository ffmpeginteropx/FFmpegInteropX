#pragma once
#include <pch.h>
#include <KeyStringValuePair.h>

namespace FFmpegInteropX
{
	using namespace winrt::Windows::Foundation;
	using namespace winrt::Windows::Foundation::Collections;
	using namespace winrt::FFmpegInteropX;

	class MediaMetadata
	{
		IVector<IKeyValuePair<winrt::hstring, winrt::hstring>> entries;
		bool tagsLoaded = false;
	public:

		MediaMetadata()
		{
			entries = winrt::single_threaded_vector<IKeyValuePair<winrt::hstring, winrt::hstring>>();
		}

		void LoadMetadataTags(AVFormatContext* m_pAvFormatCtx)
		{
			if (!tagsLoaded)
			{
				if (m_pAvFormatCtx->metadata)
				{
					AVDictionaryEntry* entry = NULL;

					do {
						entry = av_dict_get(m_pAvFormatCtx->metadata, "", entry, AV_DICT_IGNORE_SUFFIX);
						if (entry)
							entries->Append(KeyStringValuePair(
								StringUtils::Utf8ToPlatformString(entry->key),
								StringUtils::Utf8ToPlatformString(entry->value)));

					} while (entry);
				}
				tagsLoaded = true;
			}
		}


		IVectorView<IKeyValuePair<winrt::hstring, winrt::hstring>> MetadataTags()
		{
			return entries->GetView();
		}
	};
}


