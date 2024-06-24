#pragma once
#include "MediaThumbnailData.g.h"

namespace winrt::FFmpegInteropX::implementation
{
    struct MediaThumbnailData : MediaThumbnailDataT<MediaThumbnailData>
    {
        MediaThumbnailData() = default;

        MediaThumbnailData(Windows::Storage::Streams::IBuffer const& buffer, hstring const& extension);
        Windows::Storage::Streams::IBuffer Buffer();
        hstring Extension();
        void Close();

        winrt::Windows::Storage::Streams::IBuffer _buffer = { nullptr };
        winrt::hstring _extension{};
        ~MediaThumbnailData();
    };
}
namespace winrt::FFmpegInteropX::factory_implementation
{
    struct MediaThumbnailData : MediaThumbnailDataT<MediaThumbnailData, implementation::MediaThumbnailData>
    {
    };
}
