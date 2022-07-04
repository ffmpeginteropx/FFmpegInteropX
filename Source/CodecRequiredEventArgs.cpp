#include "pch.h"
#include "CodecRequiredEventArgs.h"
#include "CodecRequiredEventArgs.g.cpp"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropX::implementation
{
    FFmpegInteropX::CodecRequiredReason CodecRequiredEventArgs::Reason()
    {
        return this->reason;
    }
    hstring CodecRequiredEventArgs::FormatName()
    {
        return this->codecName;
    }
    hstring CodecRequiredEventArgs::StoreExtensionName()
    {
        return this->storeExtensionName;
    }
    hstring CodecRequiredEventArgs::ProductId()
    {
        return this->productId;
    }

    Windows::Foundation::IAsyncOperation<bool> CodecRequiredEventArgs::OpenStorePageAsync()
    {
        return winrt::Windows::System::Launcher::LaunchUriAsync(Uri(L"ms-windows-store://pdp/?ProductId=" + productId));
    }
}
