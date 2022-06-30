#pragma once
#include <pch.h>
#include "KeyStringValuePair.h"

namespace FFmpegInteropX
{
    using namespace Platform;
    using namespace Platform::Collections;
    using namespace Windows::Foundation;
    using namespace Windows::Foundation::Collections;

    ref class MediaMetadata sealed
    {
        Map<String^, IVectorView<String^>^>^ entries;
        bool tagsLoaded = false;
    internal:

        MediaMetadata()
        {
            entries = ref new Map<String^, IVectorView<String^>^>();
        }

        void LoadMetadataTags(AVFormatContext* m_pAvFormatCtx)
        {
            if (!tagsLoaded)
            {
                if (m_pAvFormatCtx->metadata)
                {
                    AVDictionaryEntry* entry = NULL;
                    auto localEntries = ref new Map<String^, IVector<String^>^>();
                    do {
                        entry = av_dict_get(m_pAvFormatCtx->metadata, "", entry, AV_DICT_IGNORE_SUFFIX);
                        if (entry)
                        {
                            auto key = StringUtils::Utf8ToPlatformString(entry->key);
                            auto value = StringUtils::Utf8ToPlatformString(entry->value);
                            if (localEntries->HasKey(key))
                            {
                                auto vector = localEntries->Lookup(key);
                                vector->Append(value);
                            }
                            else
                            {
                                auto vector = ref new Vector<String^>();
                                vector->Append(value);
                                localEntries->Insert(key, vector);
                            }
                        }


                    } while (entry);


                    for (auto kvp : localEntries)
                    {
                        entries->Insert(kvp->Key, kvp->Value->GetView());
                    }
                }
                tagsLoaded = true;
            }
        }


        property IMapView<String^, IVectorView<String^>^>^ MetadataTags
        {
            IMapView<String^, IVectorView<String^>^>^ get()
            {
                return entries->GetView();
            }
        }

    };


}


