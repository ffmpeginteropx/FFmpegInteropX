#include "pch.h"
#include "AttachedFile.h"
#include "NativeBufferFactory.h"

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
