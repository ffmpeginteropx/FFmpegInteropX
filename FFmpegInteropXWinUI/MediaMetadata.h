#pragma once
#pragma once
#include <pch.h>

namespace FFmpegInteropX
{
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Windows::Foundation::Collections;

    class MediaMetadata
    {
        IMap<winrt::hstring, IVectorView<winrt::hstring>> entries = { nullptr };
        bool tagsLoaded = false;
    public:

        MediaMetadata()
        {
            entries = winrt::single_threaded_map<winrt::hstring, IVectorView<winrt::hstring>>();
        }

        void LoadMetadataTags(AVFormatContext* m_pAvFormatCtx)
        {
            if (!tagsLoaded)
            {
                auto entriesLocal = winrt::single_threaded_map<winrt::hstring, IVector<winrt::hstring>>();
                if (m_pAvFormatCtx->metadata)
                {
                    AVDictionaryEntry* entry = NULL;

                    do {
                        entry = av_dict_get(m_pAvFormatCtx->metadata, "", entry, AV_DICT_IGNORE_SUFFIX);
                        if (entry)
                        {
                            auto key = StringUtils::Utf8ToPlatformString(entry->key);
                            auto value = StringUtils::Utf8ToPlatformString(entry->value);
                            if (entries.HasKey(key))
                            {
                                auto valueList = entriesLocal.Lookup(key);
                                valueList.Append(value);
                            }
                            else
                            {
                                auto valueList = winrt::single_threaded_vector<winrt::hstring>();
                                valueList.Append(value);
                                entriesLocal.Insert(key, valueList);
                            }
                            /*auto kvp = winrt::single_threaded_map<winrt::hstring, winrt::hstring>(StringUtils::Utf8ToPlatformString(entry->key),
                                StringUtils::Utf8ToPlatformString(entry->value))
                            entries.Append({
                                 });*/
                        }

                    } while (entry);

                    for (auto kvp : entriesLocal)
                    {
                        entries.Insert(kvp.Key(), kvp.Value().GetView());
                    }
                }
                tagsLoaded = true;
            }
        }


        IMapView<winrt::hstring, IVectorView<winrt::hstring>> MetadataTags()
        {
            return entries.GetView();
        }

    };
}


