#include "pch.h"
#include "CodecRequiredEventArgs.h"
#include "CodecRequiredEventArgs.g.cpp"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropXWinUI::implementation
{
    FFmpegInteropXWinUI::CodecRequiredReason CodecRequiredEventArgs::Reason()
    {
        throw hresult_not_implemented();
    }
    hstring CodecRequiredEventArgs::FormatName()
    {
        throw hresult_not_implemented();
    }
    hstring CodecRequiredEventArgs::StoreExtensionName()
    {
        throw hresult_not_implemented();
    }
    hstring CodecRequiredEventArgs::ProductId()
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::IAsyncOperation<bool> CodecRequiredEventArgs::OpenStorePageAsync()
    {
        throw hresult_not_implemented();
    }
}
