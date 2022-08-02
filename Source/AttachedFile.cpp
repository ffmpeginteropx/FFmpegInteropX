#include "pch.h"
#include "AttachedFile.h"
#include "NativeBufferFactory.h"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

winrt::hstring AttachedFile::Name()
{
    return this->name;
}

winrt::hstring AttachedFile::MimeType()
{
    return this->mimeType;
}

uint64_t AttachedFile::Size()
{
    return stream->codecpar->extradata_size;
}

winrt::Windows::Storage::Streams::IBuffer AttachedFile::GetBuffer()
{
    return NativeBuffer::NativeBufferFactory::CreateNativeBuffer(stream->codecpar->extradata, (DWORD)Size());
}
