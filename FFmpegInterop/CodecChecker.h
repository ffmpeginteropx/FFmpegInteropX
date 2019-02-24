#pragma once

#include <d3d11.h>
#include <mutex>

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::System;
using namespace Concurrency;

namespace FFmpegInterop
{
	public ref class CodecChecker sealed
	{

	public:

		static property bool HasHardwareAccelerationH264 { bool get() { return hasHardwareAccelerationH264; } }
		static property bool HasHardwareAccelerationHEVC { bool get() { return hasHardwareAccelerationHEVC; } }
		static property bool HasHardwareAccelerationHEVCMain10 { bool get() { return hasHardwareAccelerationHEVCMain10; } }
		static property bool HasHardwareAccelerationWMV3 { bool get() { return hasHardwareAccelerationWMV3; } }
		static property bool HasHardwareAccelerationVC1 { bool get() { return hasHardwareAccelerationVC1; } }
		static property bool HasHardwareAccelerationVP9 { bool get() { return hasHardwareAccelerationVP9; } }
		static property bool HasHardwareAccelerationVP910Bit { bool get() { return hasHardwareAccelerationVP910Bit; } }
		static property bool HasHardwareAccelerationMPEG2 { bool get() { return hasHardwareAccelerationMPEG2; } }

		static void CheckHardwareAcceleration()
		{
			if (!hasCheckedHardwareAcceleration)
			{
				mutex.lock();

				if (!hasCheckedHardwareAcceleration)
				{
					PerformCheckHardwareAcceleration();
				}

				mutex.unlock();
			}
		}

		static bool PerformCheckHardwareAcceleration()
		{
			hasHardwareAccelerationH264 = false;
			hasHardwareAccelerationHEVC = false;
			hasHardwareAccelerationHEVCMain10 = false;
			hasHardwareAccelerationWMV3 = false;
			hasHardwareAccelerationVC1 = false;
			hasHardwareAccelerationVP9 = false;
			hasHardwareAccelerationVP910Bit = false;
			hasHardwareAccelerationMPEG2 = false;

			ID3D11Device* device = NULL;
			HRESULT hr;
			hr = D3D11CreateDevice(
				NULL,
				D3D_DRIVER_TYPE_HARDWARE,
				NULL,
				0,
				NULL,
				0,
				D3D11_SDK_VERSION,
				&device,
				NULL,
				NULL);

			ID3D11VideoDevice* videoDevice = NULL;
			if (SUCCEEDED(hr))
			{
				hr = device->QueryInterface(&videoDevice);
			}

			if (SUCCEEDED(hr))
			{
				GUID profile;
				int count = videoDevice->GetVideoDecoderProfileCount();
				for (int i = 0; i < count; i++)
				{
					hr = videoDevice->GetVideoDecoderProfile(i, &profile);

					if (SUCCEEDED(hr))
					{
						if (CheckHWProfile(profile, videoDevice, hasHardwareAccelerationH264, D3D11_DECODER_PROFILE_H264_VLD_FGT)) continue;
						if (CheckHWProfile(profile, videoDevice, hasHardwareAccelerationH264, D3D11_DECODER_PROFILE_H264_VLD_NOFGT)) continue;
						if (CheckHWProfile(profile, videoDevice, hasHardwareAccelerationH264, D3D11_DECODER_PROFILE_H264_VLD_WITHFMOASO_NOFGT)) continue;

						if (CheckHWProfile(profile, videoDevice, hasHardwareAccelerationHEVC, D3D11_DECODER_PROFILE_HEVC_VLD_MAIN)) continue;
						if (CheckHWProfile(profile, videoDevice, hasHardwareAccelerationHEVC, D3D11_DECODER_PROFILE_HEVC_VLD_MAIN10)) continue;

						if (CheckHWProfile(profile, videoDevice, hasHardwareAccelerationHEVCMain10, D3D11_DECODER_PROFILE_HEVC_VLD_MAIN10)) continue;

						if (CheckHWProfile(profile, videoDevice, hasHardwareAccelerationMPEG2, D3D11_DECODER_PROFILE_MPEG2_VLD)) continue;
						if (CheckHWProfile(profile, videoDevice, hasHardwareAccelerationMPEG2, D3D11_DECODER_PROFILE_MPEG2and1_VLD)) continue;

						if (CheckHWProfile(profile, videoDevice, hasHardwareAccelerationVC1, D3D11_DECODER_PROFILE_VC1_VLD)) continue;
						if (CheckHWProfile(profile, videoDevice, hasHardwareAccelerationVC1, D3D11_DECODER_PROFILE_VC1_D2010)) continue;

						if (CheckHWProfile(profile, videoDevice, hasHardwareAccelerationVP9, D3D11_DECODER_PROFILE_VP9_VLD_PROFILE0)) continue;
						if (CheckHWProfile(profile, videoDevice, hasHardwareAccelerationVP9, D3D11_DECODER_PROFILE_VP9_VLD_10BIT_PROFILE2)) continue;

						if (CheckHWProfile(profile, videoDevice, hasHardwareAccelerationVP910Bit, D3D11_DECODER_PROFILE_VP9_VLD_10BIT_PROFILE2)) continue;
					}
				}
			}

			if (videoDevice)
			{
				videoDevice->Release();
			}

			if (device)
			{
				device->Release();
			}

			return false;
		}

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

		static std::mutex mutex;

		static bool hasCheckedHardwareAcceleration;
		static bool hasCheckedMpeg2Extension;
		static bool hasCheckedVP9Extension;

		static bool hasHardwareAccelerationH264;
		static bool hasHardwareAccelerationHEVC;
		static bool hasHardwareAccelerationHEVCMain10;
		static bool hasHardwareAccelerationWMV3;
		static bool hasHardwareAccelerationVC1;
		static bool hasHardwareAccelerationVP9;
		static bool hasHardwareAccelerationVP910Bit;
		static bool hasHardwareAccelerationMPEG2;

		static bool CheckHWProfile(GUID profile, bool& hasHardwareAcceleration, GUID* profiles, int profileCount)
		{
			if (!hasHardwareAcceleration)
			{
				for (size_t i = 0; i < profileCount; i++)
				{
					if (profiles[i] == profile)
					{
						hasHardwareAcceleration = true;
						return true;
					}
				}
			}
			return false;
		}

		inline static bool CheckHWProfile(GUID profile, ID3D11VideoDevice* videoDevice, bool& hasHardwareAcceleration, GUID hardwareAccelerationProfile)
		{
			if (!hasHardwareAcceleration && profile == hardwareAccelerationProfile)
			{
				UINT count;
				D3D11_VIDEO_DECODER_DESC desc;
				D3D11_VIDEO_DECODER_CONFIG config;
				desc.Guid = profile;
				desc.OutputFormat = DXGI_FORMAT_NV12;
				desc.SampleWidth = 8192;
				desc.SampleHeight = 4320;
				videoDevice->GetVideoDecoderConfigCount(&desc, &count);
				if (count > 0)
				{
					videoDevice->GetVideoDecoderConfig(&desc, 0, &config);
				}
				hasHardwareAcceleration = true;
				return true;
			}
			return false;
		}

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

	bool CodecChecker::hasCheckedHardwareAcceleration = false;
	bool CodecChecker::hasCheckedMpeg2Extension = false;
	bool CodecChecker::hasCheckedVP9Extension = false;
	std::mutex CodecChecker::mutex;

	bool CodecChecker::hasHardwareAccelerationH264;
	bool CodecChecker::hasHardwareAccelerationHEVC;
	bool CodecChecker::hasHardwareAccelerationHEVCMain10;
	bool CodecChecker::hasHardwareAccelerationWMV3;
	bool CodecChecker::hasHardwareAccelerationVC1;
	bool CodecChecker::hasHardwareAccelerationVP9;
	bool CodecChecker::hasHardwareAccelerationVP910Bit;
	bool CodecChecker::hasHardwareAccelerationMPEG2;

}

