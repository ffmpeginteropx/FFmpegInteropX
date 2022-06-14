#pragma once
#include "CodecRequiredEventArgs.g.h"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropXWinUI::implementation
{
	struct CodecRequiredEventArgs : CodecRequiredEventArgsT<CodecRequiredEventArgs>
	{
		CodecRequiredEventArgs() = default;

		FFmpegInteropXWinUI::CodecRequiredReason Reason();
		hstring FormatName();
		hstring StoreExtensionName();
		hstring ProductId();
		Windows::Foundation::IAsyncOperation<bool> OpenStorePageAsync();

		CodecRequiredEventArgs(CodecRequiredReason reason, hstring  const& codecName, hstring  const& storeExtensionName, hstring  const& productId)
		{
			this->reason = reason;
			this->codecName = codecName;
			this->storeExtensionName = storeExtensionName;
			this->productId = productId;
		}

	private:
		CodecRequiredReason reason;
		hstring codecName;
		hstring storeExtensionName;
		hstring productId;
	};
}
