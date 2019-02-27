#pragma once

#include <d3d11.h>
#include <mutex>

#include "libavformat/avformat.h"

using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Foundation;
using namespace Windows::System;
using namespace Concurrency;

namespace FFmpegInterop
{
	enum VideoResolution
	{
		UnknownResolution,
		SD,
		HD,
		FullHD,
		UHD4K,
		UHD8K,
	};

	ref class HardwareAccelerationStatus
	{
	internal:
		property bool IsAvailable;
		property Vector<int>^ SupportedProfiles;
		property VideoResolution VideoResolution;

		void Reset()
		{
			IsAvailable = false;
			SupportedProfiles = nullptr;
			VideoResolution = FFmpegInterop::VideoResolution::UnknownResolution;
		}
	};

	public ref class CodecChecker sealed
	{
	public:
		
		static IAsyncAction^ InitializeAsync()
		{
			return create_async(&Initialize);
		}

		static IAsyncAction^ RefreshAsync()
		{
			return create_async(&Refresh);
		}

		static IAsyncOperation<bool>^ CheckIsMpeg2VideoExtensionInstalledAsync()
		{
			hasCheckedMpeg2Extension = false;
			return create_async(&CheckIsMpeg2VideoExtensionInstalled);
		}

		static IAsyncOperation<bool>^ CheckIsVP9VideoExtensionInstalledAsync()
		{
			hasCheckedVP9Extension = false;
			return create_async(&CheckIsVP9VideoExtensionInstalled);
		}

		static IAsyncOperation<bool>^ OpenMpeg2VideoExtensionStoreEntryAsync()
		{
			hasCheckedMpeg2Extension = false;
			return create_async([] { return Launcher::LaunchUriAsync(ref new Uri("ms-windows-store://pdp/?ProductId=9n95q1zzpmh4")); });
		}

		static IAsyncOperation<bool>^ OpenVP9VideoExtensionStoreEntryAsync()
		{
			hasCheckedVP9Extension = false;
			return create_async([] { return Launcher::LaunchUriAsync(ref new Uri("ms-windows-store://pdp/?ProductId=9n4d0msmp0pt")); });
		}

	internal:

		static void Initialize()
		{
			if (!hasCheckedHardwareAcceleration)
			{
				mutex.lock();

				if (!hasCheckedHardwareAcceleration)
				{
					PerformCheckHardwareAcceleration();
					hasCheckedHardwareAcceleration = true;
				}

				mutex.unlock();
			}
		}

		static void Refresh()
		{
			mutex.lock();

			PerformCheckHardwareAcceleration();
			hasCheckedHardwareAcceleration = true;
			hasCheckedMpeg2Extension = false;
			hasCheckedVP9Extension = false;

			mutex.unlock();
		}

		static bool CheckIsMpeg2VideoExtensionInstalled()
		{
			if (!hasCheckedMpeg2Extension)
			{
				mutex.lock();

				if (!hasCheckedMpeg2Extension)
				{
					try
					{
						isMpeg2ExtensionInstalled = IsAppInstalledAsync("Microsoft.MPEG2VideoExtension_8wekyb3d8bbwe").get();
					}
					catch ( ...)
					{
						isMpeg2ExtensionInstalled = false;
					}
					hasCheckedMpeg2Extension = true;
				}

				mutex.unlock();
			}

			return isMpeg2ExtensionInstalled;
		}

		static bool CheckIsVP9VideoExtensionInstalled()
		{
			if (!hasCheckedVP9Extension)
			{
				mutex.lock();

				if (!hasCheckedVP9Extension)
				{
					try
					{
						isVP9ExtensionInstalled = IsAppInstalledAsync("Microsoft.VP9VideoExtensions_8wekyb3d8bbwe").get();
					}
					catch (...)
					{
						isVP9ExtensionInstalled = false;
					}
					hasCheckedVP9Extension = true;
				}

				mutex.unlock();
			}

			return isVP9ExtensionInstalled;
		}

		static property HardwareAccelerationStatus^ HardwareAccelerationH264 { HardwareAccelerationStatus^ get() { return hardwareAccelerationH264; } }
		static property HardwareAccelerationStatus^ HardwareAccelerationHEVC { HardwareAccelerationStatus^ get() { return hardwareAccelerationHEVC; } }
		static property HardwareAccelerationStatus^ HardwareAccelerationWMV3 { HardwareAccelerationStatus^ get() { return hardwareAccelerationWMV3; } }
		static property HardwareAccelerationStatus^ HardwareAccelerationVC1 { HardwareAccelerationStatus^ get() { return hardwareAccelerationVC1; } }
		static property HardwareAccelerationStatus^ HardwareAccelerationVP9 { HardwareAccelerationStatus^ get() { return hardwareAccelerationVP9; } }
		static property HardwareAccelerationStatus^ HardwareAccelerationMPEG2 { HardwareAccelerationStatus^ get() { return hardwareAccelerationMPEG2; } }

		static bool PerformCheckHardwareAcceleration()
		{
			hardwareAccelerationH264->Reset();
			hardwareAccelerationHEVC->Reset();
			hardwareAccelerationWMV3->Reset();
			hardwareAccelerationVC1->Reset();
			hardwareAccelerationVP9->Reset();
			hardwareAccelerationMPEG2->Reset();

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
						if (profile == D3D11_DECODER_PROFILE_H264_VLD_FGT || 
							profile == D3D11_DECODER_PROFILE_H264_VLD_NOFGT || 
							profile == D3D11_DECODER_PROFILE_H264_VLD_WITHFMOASO_NOFGT)
						{
							if (!hardwareAccelerationH264->IsAvailable)
							{
								hardwareAccelerationH264->IsAvailable = true;
								hardwareAccelerationH264->VideoResolution = CheckResolution(profile, videoDevice);
								hardwareAccelerationH264->SupportedProfiles = ref new Vector<int>();
								hardwareAccelerationH264->SupportedProfiles->Append(FF_PROFILE_H264_BASELINE);
								hardwareAccelerationH264->SupportedProfiles->Append(FF_PROFILE_H264_CONSTRAINED_BASELINE);
								hardwareAccelerationH264->SupportedProfiles->Append(FF_PROFILE_H264_EXTENDED);
								hardwareAccelerationH264->SupportedProfiles->Append(FF_PROFILE_H264_MAIN);
								hardwareAccelerationH264->SupportedProfiles->Append(FF_PROFILE_H264_HIGH);
							}
						}

						if (profile == D3D11_DECODER_PROFILE_HEVC_VLD_MAIN ||
							profile == D3D11_DECODER_PROFILE_HEVC_VLD_MAIN10)
						{
							if (!hardwareAccelerationHEVC->IsAvailable)
							{
								hardwareAccelerationHEVC->IsAvailable = true;
								hardwareAccelerationHEVC->VideoResolution = CheckResolution(profile, videoDevice);
								hardwareAccelerationHEVC->SupportedProfiles = ref new Vector<int>();
							}
							if (profile == D3D11_DECODER_PROFILE_HEVC_VLD_MAIN)
							{
								hardwareAccelerationHEVC->SupportedProfiles->Append(FF_PROFILE_HEVC_MAIN);
							}
							else
							{
								hardwareAccelerationHEVC->SupportedProfiles->Append(FF_PROFILE_HEVC_MAIN_10);
							}
						}

						if (profile == D3D11_DECODER_PROFILE_MPEG2_VLD ||
							profile == D3D11_DECODER_PROFILE_MPEG2and1_VLD)
						{
							if (!hardwareAccelerationMPEG2->IsAvailable)
							{
								hardwareAccelerationMPEG2->IsAvailable = true;
								hardwareAccelerationMPEG2->VideoResolution = CheckResolution(profile, videoDevice);
								hardwareAccelerationMPEG2->IsAvailable = true;
								hardwareAccelerationMPEG2->SupportedProfiles = ref new Vector<int>();
								hardwareAccelerationMPEG2->SupportedProfiles->Append(FF_PROFILE_MPEG2_MAIN);
								hardwareAccelerationMPEG2->SupportedProfiles->Append(FF_PROFILE_MPEG2_SIMPLE);
							}
						}

						if (profile == D3D11_DECODER_PROFILE_VC1_VLD ||
							profile == D3D11_DECODER_PROFILE_VC1_D2010)
						{
							if (!hardwareAccelerationVC1->IsAvailable)
							{
								hardwareAccelerationVC1->IsAvailable = true;
								hardwareAccelerationVC1->VideoResolution = CheckResolution(profile, videoDevice);
								hardwareAccelerationWMV3->IsAvailable = true;
								hardwareAccelerationWMV3->VideoResolution = hardwareAccelerationVC1->VideoResolution;
							}
						}

						if (profile == D3D11_DECODER_PROFILE_VP9_VLD_PROFILE0 ||
							profile == D3D11_DECODER_PROFILE_VP9_VLD_10BIT_PROFILE2)
						{
							if (!hardwareAccelerationVP9->IsAvailable)
							{
								hardwareAccelerationVP9->IsAvailable = true;
								hardwareAccelerationVP9->VideoResolution = CheckResolution(profile, videoDevice);
								hardwareAccelerationVP9->SupportedProfiles = ref new Vector<int>();
							}

							if (profile == D3D11_DECODER_PROFILE_VP9_VLD_PROFILE0)
							{
								hardwareAccelerationVP9->SupportedProfiles->Append(FF_PROFILE_VP9_0);
							}
							else
							{
								hardwareAccelerationVP9->SupportedProfiles->Append(FF_PROFILE_VP9_2);
							}
						}
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


	private:

		static std::mutex mutex;

		static bool hasCheckedHardwareAcceleration;
		static bool hasCheckedMpeg2Extension;
		static bool hasCheckedVP9Extension;

		static bool isMpeg2ExtensionInstalled;
		static bool isVP9ExtensionInstalled;

		static HardwareAccelerationStatus^ hardwareAccelerationH264;
		static HardwareAccelerationStatus^ hardwareAccelerationHEVC;
		static HardwareAccelerationStatus^ hardwareAccelerationWMV3;
		static HardwareAccelerationStatus^ hardwareAccelerationVC1;
		static HardwareAccelerationStatus^ hardwareAccelerationVP9;
		static HardwareAccelerationStatus^ hardwareAccelerationMPEG2;

		inline static VideoResolution CheckResolution(GUID profile, ID3D11VideoDevice* videoDevice)
		{
			UINT count;
			D3D11_VIDEO_DECODER_DESC desc;
			desc.Guid = profile;
			desc.OutputFormat = DXGI_FORMAT_NV12;

			desc.SampleWidth = 8192;
			desc.SampleHeight = 4320;
			videoDevice->GetVideoDecoderConfigCount(&desc, &count);
			if (count > 0)
			{
				return VideoResolution::UHD8K;
			}

			desc.SampleWidth = 4096;
			desc.SampleHeight = 2160;
			videoDevice->GetVideoDecoderConfigCount(&desc, &count);
			if (count > 0)
			{
				return VideoResolution::UHD4K;
			}

			desc.SampleWidth = 1920;
			desc.SampleHeight = 1088;
			videoDevice->GetVideoDecoderConfigCount(&desc, &count);
			if (count > 0)
			{
				return VideoResolution::FullHD;
			}

			desc.SampleWidth = 1280;
			desc.SampleHeight = 720;
			videoDevice->GetVideoDecoderConfigCount(&desc, &count);
			if (count > 0)
			{
				return VideoResolution::HD;
			}

			desc.SampleWidth = 720;
			desc.SampleHeight = 576;
			videoDevice->GetVideoDecoderConfigCount(&desc, &count);
			if (count > 0)
			{
				return VideoResolution::SD;
			}

			return VideoResolution::UnknownResolution;
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

}

