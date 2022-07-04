#include "pch.h"
#include "AttachedFile.h"
#include "AttachedFile.g.cpp"
#include "NativeBufferFactory.h"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropX::implementation
{
    hstring AttachedFile::Name()
    {
        return this->name;
    }

    hstring AttachedFile::MimeType()
    {
        throw this->mimeType;
    }

    uint64_t AttachedFile::Size()
    {
        return stream->codecpar->extradata_size;
    }

    winrt::Windows::Storage::Streams::IBuffer AttachedFile::GetBuffer()
    {
        return NativeBuffer::NativeBufferFactory::CreateNativeBuffer(stream->codecpar->extradata, (DWORD)Size());
    }
}
