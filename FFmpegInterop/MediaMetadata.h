#pragma once
#include <pch.h>
#include <vector>
#include <collection.h>
extern "C"
{
#include <libavformat/avformat.h>
}

using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;

namespace FFmpegInterop {

	public ref class MediaMetadataEntry sealed
	{
		String ^key, ^value;

	public:

		property String^ Key
		{
			String^ get()
			{
				return key;
			}
		}

		property String^ Value
		{
			String^ get()
			{
				return value;
			}
		}

		MediaMetadataEntry(String^ key, String^ value)
		{
			this->key = key;
			this->value = value;
		}
	};



	public ref class MediaMetadata sealed
	{
		Vector<MediaMetadataEntry^>^ entries;

	internal:

		MediaMetadata(AVFormatContext *m_pAvFormatCtx)
		{
			entries = ref new Vector<MediaMetadataEntry^>();
			if (m_pAvFormatCtx->metadata)
			{
				AVDictionaryEntry* entry = NULL;

				do {
					entry = av_dict_get(m_pAvFormatCtx->metadata, "", entry, AV_DICT_IGNORE_SUFFIX);
					if (entry)
						entries->Append(ref new MediaMetadataEntry(StringUtils::AnsiStringToPlatformString(entry->key), StringUtils::AnsiStringToPlatformString(entry->value)));

				} while (entry);

			}
		}
	public:

		property IVectorView<MediaMetadataEntry^>^ Values
		{
			IVectorView<MediaMetadataEntry^>^ get()
			{
				return entries->GetView();
			}
		}

		virtual ~MediaMetadata() 
		{
			entries->Clear();
		}
	};


}


