#include "pch.h"
#include "AttachedFile.h"
#include "AttachedFile.g.cpp"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropXWinUI::implementation
{
    hstring AttachedFile::Name()
    {
        throw hresult_not_implemented();
    }
    hstring AttachedFile::MimeType()
    {
        throw hresult_not_implemented();
    }
    uint64_t AttachedFile::Size()
    {
        throw hresult_not_implemented();
    }

    winrt::Windows::Storage::Streams::IBuffer AttachedFile::GetBuffer()
    {
        throw hresult_not_implemented();
        //return NativeBuffer::NativeBufferFactory::CreateNativeBuffer(stream->codecpar->extradata, (DWORD)Size);
    }
}
