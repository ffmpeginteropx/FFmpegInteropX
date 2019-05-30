#pragma once
#include <pch.h>
#include <vector>
#include <collection.h>
#include "KeyStringValuePair.h"

extern "C"
{
#include <libavformat/avformat.h>
}

using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;

namespace FFmpegInterop {

	ref class MediaMetadata sealed
	{
		Vector<IKeyValuePair<String^, String^>^>^ entries;
		bool tagsLoaded = false;
	internal:

		MediaMetadata()
		{

		}

		void LoadMetadataTags(AVFormatContext *m_pAvFormatCtx)
		{
			if (!tagsLoaded) 
			{
				entries = ref new Vector<IKeyValuePair<String^, String^>^>();
				if (m_pAvFormatCtx->metadata)
				{
					AVDictionaryEntry* entry = NULL;

					do {
						entry = av_dict_get(m_pAvFormatCtx->metadata, "", entry, AV_DICT_IGNORE_SUFFIX);
						if (entry)
							entries->Append(ref new KeyStringValuePair(StringUtils::AnsiStringToPlatformString(entry->key), StringUtils::AnsiStringToPlatformString(entry->value)));

					} while (entry);
				}
				tagsLoaded = true;
			}			
		}
	

		property IVectorView<IKeyValuePair<String^, String^>^>^ MetadataTags
		{
			IVectorView<IKeyValuePair<String^, String^>^>^ get()
			{
				return entries->GetView();
			}
		}
	
	public:
		virtual ~MediaMetadata()
		{
			entries->Clear();
		}
	};


}


