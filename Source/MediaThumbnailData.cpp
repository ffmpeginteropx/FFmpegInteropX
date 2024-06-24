#include "pch.h"
#include "MediaThumbnailData.h"
#include "MediaThumbnailData.g.cpp"

namespace winrt::FFmpegInteropX::implementation
{
    MediaThumbnailData::MediaThumbnailData(Windows::Storage::Streams::IBuffer const& buffer, hstring const& extension)
    {
        this->_buffer = buffer;
        this->_extension = extension;
    }

    Windows::Storage::Streams::IBuffer MediaThumbnailData::Buffer()
    {
        return _buffer;
    }

    hstring MediaThumbnailData::Extension()
    {
        return _extension;
    }

    MediaThumbnailData:: ~MediaThumbnailData()
    {
        Close();
    }

    void MediaThumbnailData::Close()
    {
        _buffer = nullptr;
    }
}
