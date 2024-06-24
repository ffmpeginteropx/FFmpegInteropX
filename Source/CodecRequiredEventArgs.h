#pragma once
#include "CodecRequiredEventArgs.g.h"

namespace winrt::FFmpegInteropX::implementation
{
    struct CodecRequiredEventArgs : CodecRequiredEventArgsT<CodecRequiredEventArgs>
    {
        CodecRequiredEventArgs(CodecRequiredReason reason, hstring const& codecName, hstring const& storeExtensionName, hstring const& productId)
        {
            this->reason = reason;
            this->codecName = codecName;
            this->storeExtensionName = storeExtensionName;
            this->productId = productId;
        }

        FFmpegInteropX::CodecRequiredReason Reason();
        hstring FormatName();
        hstring StoreExtensionName();
        hstring ProductId();
        Windows::Foundation::IAsyncOperation<bool> OpenStorePageAsync();

    private:
        CodecRequiredReason reason;
        hstring codecName{};
        hstring storeExtensionName{};
        hstring productId{};
    };
}
namespace winrt::FFmpegInteropX::factory_implementation
{
    struct CodecRequiredEventArgs : CodecRequiredEventArgsT<CodecRequiredEventArgs, implementation::CodecRequiredEventArgs>
    {
    };
}
