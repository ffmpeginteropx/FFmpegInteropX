#pragma once
#include "AttachedFile.g.h"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropXWinUI::implementation
{
    struct AttachedFile : AttachedFileT<AttachedFile>
    {
        AttachedFile() = default;

        hstring Name();
        hstring MimeType();
        uint64_t Size();

        winrt::Windows::Storage::Streams::IBuffer GetBuffer();
    };
}
