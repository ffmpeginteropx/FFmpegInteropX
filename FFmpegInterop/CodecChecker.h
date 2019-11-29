#pragma once

#include <d3d11.h>
#include <mutex>
#include <pplawait.h>

#include "libavformat/avformat.h"

namespace FFmpegInterop
{
	using namespace Platform;
	using namespace Platform::Collections;
	using namespace Windows::Foundation;
	using namespace Windows::Foundation::Collections;
	using namespace Windows::Media::Core;
	using namespace Windows::System;
	using namespace Concurrency;

	enum VideoResolution
	{
		UnknownResolution,
		SD,
		HD,
		FullHD,
		UHD4K,
		UHD4K_DCI,
		UHD8K,
		UHD8K_DCI,
	};

	ref class HardwareAccelerationStatus
	{
	internal:
		property bool IsAvailable;
		property std::vector<std::pair<int, VideoResolution>> SupportedProfiles;
		property VideoResolution MaxResolution;

		void Reset()
		{
			IsAvailable = false;
			SupportedProfiles.clear();
			MaxResolution = VideoResolution::UnknownResolution;
		}

		void AppendProfile(int profile)
		{
			SupportedProfiles.push_back(std::pair<int, VideoResolution>(profile, VideoResolution::UnknownResolution));
		}

		void AppendProfile(int profile, VideoResolution resolution)
		{
			SupportedProfiles.push_back(std::pair<int, VideoResolution>(profile, resolution));
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

		static IAsyncOperation<bool>^ CheckIsHEVCVideoExtensionInstalledAsync()
		{
			hasCheckedHEVCExtension = false;
			return create_async(&CheckIsHEVCVideoExtensionInstalled);
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

		static IAsyncOperation<bool>^ OpenHEVCVideoExtensionStoreEntryAsync()
		{
			hasCheckedHEVCExtension = false;
#ifdef _M_AMD64
			// open free x64 extension
			return create_async([] { return Launcher::LaunchUriAsync(ref new Uri("ms-windows-store://pdp/?ProductId=9n4wgh0z6vhq")); });
#else
			// open paid extension for all other platforms
			return create_async([] { return Launcher::LaunchUriAsync(ref new Uri("ms-windows-store://pdp/?ProductId=9nmzlz57r3t7")); });
#endif // _WIN64

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
			return CheckIsVideoExtensionInstalled(
				"Microsoft.MPEG2VideoExtension_8wekyb3d8bbwe", 
				hasCheckedMpeg2Extension, isMpeg2ExtensionInstalled);
		}

		static bool CheckIsVP9VideoExtensionInstalled()
		{
			return CheckIsVideoExtensionInstalled(
				"Microsoft.VP9VideoExtensions_8wekyb3d8bbwe",
				hasCheckedVP9Extension, isVP9ExtensionInstalled);
		}

		static bool CheckIsHEVCVideoExtensionInstalled()
		{
			return CheckIsVideoExtensionInstalled(
				"Microsoft.HEVCVideoExtension_8wekyb3d8bbwe",
				hasCheckedHEVCExtension, isHEVCExtensionInstalled);
		}

		static bool CheckIsVideoExtensionInstalled(String^ appId, bool& hasCheckedExtension, bool& isExtensionInstalled)
		{
			if (!hasCheckedExtension)
			{
				mutex.lock();

				if (!hasCheckedExtension)
				{
					try
					{
						isExtensionInstalled = IsAppInstalledAsync(appId).get();
						//isExtensionInstalled = IsVideoCodecInstalledAsync(codecSubtype).get();
					}
					catch (...)
					{
						isExtensionInstalled = false;
					}
					hasCheckedExtension = true;
				}

				mutex.unlock();
			}

			return isExtensionInstalled;
		}

		static property HardwareAccelerationStatus^ HardwareAccelerationH264 { HardwareAccelerationStatus^ get() { return hardwareAccelerationH264; } }
		static property HardwareAccelerationStatus^ HardwareAccelerationHEVC { HardwareAccelerationStatus^ get() { return hardwareAccelerationHEVC; } }
		static property HardwareAccelerationStatus^ HardwareAccelerationWMV3 { HardwareAccelerationStatus^ get() { return hardwareAccelerationWMV3; } }
		static property HardwareAccelerationStatus^ HardwareAccelerationVC1 { HardwareAccelerationStatus^ get() { return hardwareAccelerationVC1; } }
		static property HardwareAccelerationStatus^ HardwareAccelerationVP9 { HardwareAccelerationStatus^ get() { return hardwareAccelerationVP9; } }
		static property HardwareAccelerationStatus^ HardwareAccelerationVP8 { HardwareAccelerationStatus^ get() { return hardwareAccelerationVP8; } }
		static property HardwareAccelerationStatus^ HardwareAccelerationMPEG2 { HardwareAccelerationStatus^ get() { return hardwareAccelerationMPEG2; } }

		static void PerformCheckHardwareAcceleration()
		{
			hardwareAccelerationH264->Reset();
			hardwareAccelerationHEVC->Reset();
			hardwareAccelerationWMV3->Reset();
			hardwareAccelerationVC1->Reset();
			hardwareAccelerationVP8->Reset();
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
								hardwareAccelerationH264->MaxResolution = CheckResolution(profile, videoDevice);
								hardwareAccelerationH264->AppendProfile(FF_PROFILE_H264_BASELINE);
								hardwareAccelerationH264->AppendProfile(FF_PROFILE_H264_CONSTRAINED_BASELINE);
								hardwareAccelerationH264->AppendProfile(FF_PROFILE_H264_EXTENDED);
								hardwareAccelerationH264->AppendProfile(FF_PROFILE_H264_MAIN);
								hardwareAccelerationH264->AppendProfile(FF_PROFILE_H264_HIGH);
							}
							continue;
						}

						if (profile == D3D11_DECODER_PROFILE_HEVC_VLD_MAIN ||
							profile == D3D11_DECODER_PROFILE_HEVC_VLD_MAIN10)
						{
							hardwareAccelerationHEVC->IsAvailable = true;

							auto resolution = CheckResolution(profile, videoDevice);
							hardwareAccelerationHEVC->MaxResolution = max(resolution, hardwareAccelerationHEVC->MaxResolution);

							if (profile == D3D11_DECODER_PROFILE_HEVC_VLD_MAIN)
							{
								hardwareAccelerationHEVC->AppendProfile(FF_PROFILE_HEVC_MAIN, resolution);
							}
							else
							{
								hardwareAccelerationHEVC->AppendProfile(FF_PROFILE_HEVC_MAIN_10, resolution);
							}
							continue;
						}

						if (profile == D3D11_DECODER_PROFILE_MPEG2_VLD ||
							profile == D3D11_DECODER_PROFILE_MPEG2and1_VLD)
						{
							if (!hardwareAccelerationMPEG2->IsAvailable)
							{
								hardwareAccelerationMPEG2->IsAvailable = true;
								hardwareAccelerationMPEG2->AppendProfile(FF_PROFILE_MPEG2_MAIN);
								hardwareAccelerationMPEG2->AppendProfile(FF_PROFILE_MPEG2_SIMPLE);
							}

							auto resolution = CheckResolution(profile, videoDevice);
							hardwareAccelerationMPEG2->MaxResolution = max(resolution, hardwareAccelerationMPEG2->MaxResolution);
							continue;
						}

						if (profile == D3D11_DECODER_PROFILE_VC1_VLD ||
							profile == D3D11_DECODER_PROFILE_VC1_D2010)
						{
							auto resolution = CheckResolution(profile, videoDevice);

							hardwareAccelerationVC1->IsAvailable = true;
							hardwareAccelerationVC1->MaxResolution = max(resolution, hardwareAccelerationVC1->MaxResolution);

							hardwareAccelerationWMV3->IsAvailable = true;
							hardwareAccelerationWMV3->MaxResolution = hardwareAccelerationVC1->MaxResolution;
							continue;
						}

						if (profile == D3D11_DECODER_PROFILE_VP9_VLD_PROFILE0 ||
							profile == D3D11_DECODER_PROFILE_VP9_VLD_10BIT_PROFILE2)
						{
							hardwareAccelerationVP9->IsAvailable = true;

							auto resolution = CheckResolution(profile, videoDevice);
							hardwareAccelerationVP9->MaxResolution = max(resolution, hardwareAccelerationVP9->MaxResolution);

							if (profile == D3D11_DECODER_PROFILE_VP9_VLD_PROFILE0)
							{
								hardwareAccelerationVP9->AppendProfile(FF_PROFILE_VP9_0, resolution);
							}
							else
							{
								hardwareAccelerationVP9->AppendProfile(FF_PROFILE_VP9_2, resolution);
							}
							continue;
						}

						if (profile == D3D11_DECODER_PROFILE_VP8_VLD)
						{
							hardwareAccelerationVP8->IsAvailable = true;

							auto resolution = CheckResolution(profile, videoDevice);
							hardwareAccelerationVP8->MaxResolution = resolution;
							continue;
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
		}

		static bool CheckUseHardwareAcceleration(HardwareAccelerationStatus^ status, AVCodecID codecId, int profile, int width, int height)
		{
			bool result = false;

			// auto detection
			if (status->IsAvailable)
			{
				// check profile, if restricted
				if (status->SupportedProfiles.size() > 0)
				{
					for each (auto profileInfo in status->SupportedProfiles)
					{
						if (profileInfo.first == profile)
						{
							result = true;

							// check profile resolution, if restricted
							if (profileInfo.second > 0)
							{
								CheckVideoResolution(result, width, height, profileInfo.second);
							}

							break;
						}
					}
				}
				else
				{
					result = true;
				}

				// check installation status of extension
				if (codecId == AV_CODEC_ID_MPEG2VIDEO)
				{
					result &= CheckIsMpeg2VideoExtensionInstalled();
				}
				else if (codecId == AV_CODEC_ID_VP9 || codecId == AV_CODEC_ID_VP8)
				{
					result &= CheckIsVP9VideoExtensionInstalled();
				}
				else if (codecId == AV_CODEC_ID_HEVC)
				{
					result &= CheckIsHEVCVideoExtensionInstalled();
				}

				CheckVideoResolution(result, width, height, status->MaxResolution);
			}

			return result;
		}

	private:

		static std::mutex mutex;

		static bool hasCheckedHardwareAcceleration;
		static bool hasCheckedMpeg2Extension;
		static bool hasCheckedVP9Extension;
		static bool hasCheckedHEVCExtension;

		static bool isMpeg2ExtensionInstalled;
		static bool isVP9ExtensionInstalled;
		static bool isHEVCExtensionInstalled;

		static HardwareAccelerationStatus^ hardwareAccelerationH264;
		static HardwareAccelerationStatus^ hardwareAccelerationHEVC;
		static HardwareAccelerationStatus^ hardwareAccelerationWMV3;
		static HardwareAccelerationStatus^ hardwareAccelerationVC1;
		static HardwareAccelerationStatus^ hardwareAccelerationVP9;
		static HardwareAccelerationStatus^ hardwareAccelerationVP8;
		static HardwareAccelerationStatus^ hardwareAccelerationMPEG2;

		inline static VideoResolution CheckResolution(GUID profile, ID3D11VideoDevice* videoDevice)
		{
			UINT count;
			D3D11_VIDEO_DECODER_DESC desc;
			desc.Guid = profile;
			desc.OutputFormat = DXGI_FORMAT_NV12;

			desc.SampleWidth = 7680;
			desc.SampleHeight = 4320;
			videoDevice->GetVideoDecoderConfigCount(&desc, &count);
			if (count > 0)
			{
				desc.SampleWidth = 8192;
				videoDevice->GetVideoDecoderConfigCount(&desc, &count);
				if (count > 0)
				{
					return VideoResolution::UHD8K_DCI;
				}
				else
				{
					return VideoResolution::UHD8K;
				}
			}

			desc.SampleWidth = 3840;
			desc.SampleHeight = 2160;
			videoDevice->GetVideoDecoderConfigCount(&desc, &count);
			if (count > 0)
			{
				desc.SampleWidth = 4096;
				videoDevice->GetVideoDecoderConfigCount(&desc, &count);
				if (count > 0)
				{
					return VideoResolution::UHD4K_DCI;
				}
				else
				{
					return VideoResolution::UHD4K;
				}
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

		static void CheckVideoResolution(bool& result, int width, int height, VideoResolution maxResolution)
		{
			if (result && width > 0 && height > 0)
			{
				switch (maxResolution)
				{
				case VideoResolution::SD:
					result = width <= 720 && height <= 576;
					break;
				case VideoResolution::HD:
					result = width <= 1280 && height <= 720;
					break;
				case VideoResolution::FullHD:
					result = width <= 1920 && height <= 1088;
					break;
				case VideoResolution::UHD4K:
					result = width <= 3840 && height <= 2160;
					break;
				case VideoResolution::UHD4K_DCI:
					result = width <= 4096 && height <= 2160;
					break;
				case VideoResolution::UHD8K:
					result = width <= 7680 && height <= 4320;
					break;
				case VideoResolution::UHD8K_DCI:
					result = width <= 8192 && height <= 4320;
					break;
				default:
					break;
				}
			}
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

		//// this works, but it takes 500ms on first call, so not using it right now...
		//static task<bool> IsVideoCodecInstalledAsync(String^ videoCodecSubtype)
		//{
		//	auto query = ref new CodecQuery();
		//	auto codecs = co_await query->FindAllAsync(CodecKind::Video, CodecCategory::Decoder, videoCodecSubtype);
		//	for each (auto codec in codecs)
		//	{
		//		if (codec->IsTrusted)
		//		{
		//			return true;
		//		}
		//	}
		//	return false;
		//}

	};

}

