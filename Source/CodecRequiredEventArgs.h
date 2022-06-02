#pragma once
#include "CodecRequiredEventArgs.g.h"

namespace winrt::FFmpegInteropX::implementation
{
	
	using namespace Windows::Foundation;

	///<summary>Specifies the reason why a codec extension installation is recommended.</summary>
	enum CodecRequiredReason
	{
		///<summary>Unknown.</summary>
		Unknown = 0x00,

		///<summary>The codec extension will allow hardware acceleration of a specific format.</summary>
		HardwareAcceleration = 0x01
	};

	struct CodecRequiredEventArgs : CodecRequiredEventArgsT< CodecRequiredEventArgs>
	{
	public:
		///<summary>The reason why a new codec extension is recommended.</summary>
		CodecRequiredReason Reason() { return reason; }

		///<summary>The name of the video or audio format (e.g. "HEVC" or "VP9").</summary>
		hstring FormatName() { return codecName; }

		///<summary>The non-localized name of the proposed extension in the Windows Store.</summary>
		hstring StoreExtensionName() { return storeExtensionName; }

		///<summary>The ProductId of the proposed extension.</summary>
		hstring ProductId() { return productId; }

		///<summary>This will open the Windows Store page of the proposed codec extension.</summary>
		IAsyncOperation<bool> OpenStorePageAsync()
		{
			return winrt::Windows::System::Launcher::LaunchUriAsync(Uri(L"ms-windows-store://pdp/?ProductId=" + productId));
		}

		CodecRequiredEventArgs(CodecRequiredReason reason, hstring codecName, hstring storeExtensionName, hstring productId)
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

namespace winrt::FFmpegInteropX::factory_implementation
{
	struct CodecRequiredEventArgs : CodecRequiredEventArgsT<CodecRequiredEventArgs, implementation::CodecRequiredEventArgs>
	{
	};
}