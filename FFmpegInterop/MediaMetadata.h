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
		
	public ref class MediaMetadata sealed
	{
		Vector<KeyStringValuePair^>^ entries;

	internal:

		MediaMetadata(AVFormatContext *m_pAvFormatCtx)
		{
			entries = ref new Vector<KeyStringValuePair^>();
			if (m_pAvFormatCtx->metadata)
			{
				AVDictionaryEntry* entry = NULL;

				do {
					entry = av_dict_get(m_pAvFormatCtx->metadata, "", entry, AV_DICT_IGNORE_SUFFIX);
					if (entry)
						entries->Append(ref new KeyStringValuePair(StringUtils::AnsiStringToPlatformString(entry->key), StringUtils::AnsiStringToPlatformString(entry->value)));

				} while (entry);

			}
		}
	public:

		property IVectorView<KeyStringValuePair^>^ Values
		{
			IVectorView<KeyStringValuePair^>^ get()
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


