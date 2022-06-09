#pragma once

namespace FFmpegInteropX
{
	using namespace Platform;
	using namespace Windows::Foundation;

	///<summary>Specifies the reason why a codec extension installation is recommended.</summary>
	public enum class CodecRequiredReason
	{
		///<summary>Unknown.</summary>
		Unknown = 0x00,

		///<summary>The codec extension will allow hardware acceleration of a specific format.</summary>
		HardwareAcceleration = 0x01
	};

	public ref class CodecRequiredEventArgs sealed
	{
	public:
		///<summary>The reason why a new codec extension is recommended.</summary>
		property CodecRequiredReason Reason { CodecRequiredReason get() { return reason; } }
		
		///<summary>The name of the video or audio format (e.g. "HEVC" or "VP9").</summary>
		property String^ FormatName { String^ get() { return codecName; } }
		
		///<summary>The non-localized name of the proposed extension in the Windows Store.</summary>
		property String^ StoreExtensionName { String^ get() { return storeExtensionName; } }
		
		///<summary>The ProductId of the proposed extension.</summary>
		property String^ ProductId { String^ get() { return productId; } }

		///<summary>This will open the Windows Store page of the proposed codec extension.</summary>
		IAsyncOperation<bool>^ OpenStorePageAsync()
		{
			return Windows::System::Launcher::LaunchUriAsync(ref new Uri("ms-windows-store://pdp/?ProductId=" + productId));
		}

	internal:
		CodecRequiredEventArgs(CodecRequiredReason reason, String^ codecName, String^ storeExtensionName, String^ productId)
		{
			this->reason = reason;
			this->codecName = codecName;
			this->storeExtensionName = storeExtensionName;
			this->productId = productId;
		}

	private:
		CodecRequiredReason reason;
		String^ codecName;
		String^ storeExtensionName;
		String^ productId;
	};

}