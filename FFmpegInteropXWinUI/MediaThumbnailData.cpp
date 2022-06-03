#include "pch.h"
#include "MediaThumbnailData.h"
#include "MediaThumbnailData.g.cpp"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropXWinUI::implementation
{
    MediaThumbnailData::MediaThumbnailData(Windows::Storage::Streams::IBuffer const& buffer, hstring const& extension)
    {
        throw hresult_not_implemented();
    }
    Windows::Storage::Streams::IBuffer MediaThumbnailData::Buffer()
    {
        throw hresult_not_implemented();
    }
    hstring MediaThumbnailData::Extension()
    {
        throw hresult_not_implemented();
    }
}
