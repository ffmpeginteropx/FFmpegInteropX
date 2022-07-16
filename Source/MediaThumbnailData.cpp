#include "pch.h"
#include "MediaThumbnailData.h"
#include "MediaThumbnailData.g.cpp"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

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
