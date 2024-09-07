#include "pch.h"
#include "CodecRequiredEventArgs.h"
#include "CodecRequiredEventArgs.g.cpp"

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
