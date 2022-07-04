#pragma once
#include "AttachedFile.g.h"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropX::implementation
{
    struct AttachedFile : AttachedFileT<AttachedFile>
    {
        AttachedFile() = default;

        hstring Name();
        hstring MimeType();
        uint64_t Size();

    public:
        winrt::Windows::Storage::Streams::IBuffer GetBuffer();

        AttachedFile(hstring  const& name, hstring  const& mimeType, AVStream* stream)
        {
            this->name = name;
            this->mimeType = mimeType;
            this->stream = stream;
        }

    private:
        hstring name{};
        hstring mimeType{};

        AVStream* stream = NULL;
    };
}
