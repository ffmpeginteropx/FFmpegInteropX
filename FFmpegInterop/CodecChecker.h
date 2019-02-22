#pragma once

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::System;
using namespace Concurrency;

namespace FFmpegInterop
{
	public ref class CodecChecker sealed
	{

	public:

		static IAsyncOperation<bool>^ CheckIsMpeg2VideoExtensionInstalledAsync()
		{
			return create_async([] { return IsAppInstalledAsync("Microsoft.MPEG2VideoExtension_8wekyb3d8bbwe"); });
		}

		static IAsyncOperation<bool>^ CheckIsVP9VideoExtensionInstalledAsync()
		{
			return create_async([] { return IsAppInstalledAsync("Microsoft.VP9VideoExtensions_8wekyb3d8bbwe"); });
		}

		static IAsyncOperation<bool>^ OpenMpeg2VideoExtensionStoreEntryAsync()
		{
			return create_async([] { return Launcher::LaunchUriAsync(ref new Uri("ms-windows-store://pdp/?ProductId=9n95q1zzpmh4")); });
		}

		static IAsyncOperation<bool>^ OpenVP9VideoExtensionStoreEntryAsync()
		{
			return create_async([] { return Launcher::LaunchUriAsync(ref new Uri("ms-windows-store://pdp/?ProductId=9n4d0msmp0pt")); });
		}

	private:

		static task<bool> IsAppInstalledAsync(String^ packageName)
		{
			using namespace Windows::System;
			return create_task(Launcher::QueryUriSupportAsync(ref new Uri("mailto:dummy@mail.com"), LaunchQuerySupportType::Uri, packageName)).then(
				[](task<LaunchQuerySupportStatus> t)
			{
				try
				{
					switch (t.get())
					{
					case LaunchQuerySupportStatus::Available:
					case LaunchQuerySupportStatus::NotSupported:
						return true;
						//case LaunchQuerySupportStatus.AppNotInstalled:
						//case LaunchQuerySupportStatus.AppUnavailable:
						//case LaunchQuerySupportStatus.Unknown:
					default:
						return false;
					}
				}
				catch (...)
				{
				}
				return false;
			});
		}

	};

}

