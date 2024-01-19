#pragma once
#include "pch.h"
#include <vector>
#include <map>
#include <winrt/Windows.Media.Core.h>

using namespace winrt::Windows::Media::Core;


class MediaSampleResult
{
public:
    const MediaStreamSample sample = { nullptr };
    const std::vector< std::pair<GUID, winrt::Windows::Foundation::IInspectable>> formatChanges;


    MediaSampleResult(winrt::Windows::Media::Core::MediaStreamSample const& m_sample,
        std::vector< std::pair<GUID, winrt::Windows::Foundation::IInspectable>>& m_formatChanges)
        :sample(m_sample), formatChanges(m_formatChanges)
    {

    }
};

