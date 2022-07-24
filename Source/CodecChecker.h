#pragma once
#include "CodecChecker.g.h"
#include "pch.h"
#include "CodecRequiredEventArgs.h"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropX::implementation
{
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

    class HardwareAccelerationStatus
    {
    public:
        bool IsAvailable = false;
        std::vector<std::pair<int, VideoResolution>> SupportedProfiles;
        VideoResolution MaxResolution = VideoResolution::UnknownResolution;

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

    struct CodecChecker
    {
        ///<summary>This event is raised if a codec is required to improve playback experience.</summary>
        ///<remarks>The event is only raised once per codec. It will be raised again after a call to RefreshAsync().</remarks>
        static winrt::event_token CodecRequired(winrt::Windows::Foundation::EventHandler<winrt::FFmpegInteropX::CodecRequiredEventArgs> const& handler);
        static void CodecRequired(winrt::event_token const& token) noexcept;

        ///<summary>This will pre-initialize the hardware acceleration status.</summary>
        ///<remarks>This can be called on app startup, but it is not required.</remarks>
        static Windows::Foundation::IAsyncAction InitializeAsync();

        ///<summary>This will refresh the hardware acceleration status.</summary>
        ///<remarks>Call this after installing a codec or after a change of the active GPU.</remarks>
        static Windows::Foundation::IAsyncAction RefreshAsync();

        static Windows::Foundation::IAsyncOperation<bool> CheckIsMpeg2VideoExtensionInstalledAsync();
        static Windows::Foundation::IAsyncOperation<bool> CheckIsVP9VideoExtensionInstalledAsync();
        static Windows::Foundation::IAsyncOperation<bool> CheckIsHEVCVideoExtensionInstalledAsync();
        static Windows::Foundation::IAsyncOperation<bool> OpenMpeg2VideoExtensionStoreEntryAsync();
        static Windows::Foundation::IAsyncOperation<bool> OpenVP9VideoExtensionStoreEntryAsync();
        static Windows::Foundation::IAsyncOperation<bool> OpenHEVCVideoExtensionStoreEntryAsync();

        //internal:

        static void Initialize()
        {
            if (!hasCheckedHardwareAcceleration)
            {
                std::lock_guard lock(mutex);

                if (!hasCheckedHardwareAcceleration)
                {
                    PerformCheckHardwareAcceleration();
                    hasCheckedHardwareAcceleration = true;
                }
            }
        }

        static void Refresh()
        {
            std::lock_guard lock(mutex);

            PerformCheckHardwareAcceleration();
            hasCheckedHardwareAcceleration = true;

            hasCheckedMpeg2Extension = false;
            hasCheckedVP9Extension = false;
            hasCheckedHEVCExtension = false;

            hasAskedInstallMpeg2Extension = false;
            hasAskedInstallVP9Extension = false;
            hasAskedInstallHEVCExtension = false;
        }

        static bool CheckIsMpeg2VideoExtensionInstalled()
        {
            return CheckIsVideoExtensionInstalled(
                L"Microsoft.MPEG2VideoExtension_8wekyb3d8bbwe",
                hasCheckedMpeg2Extension, isMpeg2ExtensionInstalled);
        }

        static bool CheckIsVP9VideoExtensionInstalled()
        {
            return CheckIsVideoExtensionInstalled(
                L"Microsoft.VP9VideoExtensions_8wekyb3d8bbwe",
                hasCheckedVP9Extension, isVP9ExtensionInstalled);
        }

        static bool CheckIsHEVCVideoExtensionInstalled()
        {
            return CheckIsVideoExtensionInstalled(
                L"Microsoft.HEVCVideoExtension_8wekyb3d8bbwe",
                hasCheckedHEVCExtension, isHEVCExtensionInstalled);
        }

        static bool CheckIsVideoExtensionInstalled(hstring const& appId, bool& hasCheckedExtension, bool& isExtensionInstalled)
        {
            if (!hasCheckedExtension)
            {
                std::lock_guard lock(mutex);

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
            }

            return isExtensionInstalled;
        }

        static void RaiseCodecRequired(CodecRequiredReason reason, hstring const& codecName, hstring const& storeEntryName, hstring const& uri)
        {
            FFmpegInteropX::CodecRequiredEventArgs args(reason, codecName, storeEntryName, uri);
            m_codecRequiredEvent(winrt::Windows::Foundation::IInspectable(), args);
        }

        static  HardwareAccelerationStatus HardwareAccelerationH264() { return hardwareAccelerationH264; };

        static  HardwareAccelerationStatus HardwareAccelerationHEVC() { return hardwareAccelerationHEVC; }
        static  HardwareAccelerationStatus HardwareAccelerationWMV3() { return hardwareAccelerationWMV3; }
        static  HardwareAccelerationStatus HardwareAccelerationVC1() { return hardwareAccelerationVC1; }
        static  HardwareAccelerationStatus HardwareAccelerationVP9() { return hardwareAccelerationVP9; }
        static  HardwareAccelerationStatus HardwareAccelerationVP8() { return hardwareAccelerationVP8; }
        static  HardwareAccelerationStatus HardwareAccelerationMPEG2() { return hardwareAccelerationMPEG2; }

        static void PerformCheckHardwareAcceleration()
        {
            hardwareAccelerationH264.Reset();
            hardwareAccelerationHEVC.Reset();
            hardwareAccelerationWMV3.Reset();
            hardwareAccelerationVC1.Reset();
            hardwareAccelerationVP8.Reset();
            hardwareAccelerationVP9.Reset();
            hardwareAccelerationMPEG2.Reset();

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
                            if (!hardwareAccelerationH264.IsAvailable)
                            {
                                hardwareAccelerationH264.IsAvailable = true;
                                hardwareAccelerationH264.MaxResolution = CheckResolution(profile, videoDevice);
                                hardwareAccelerationH264.AppendProfile(FF_PROFILE_H264_BASELINE);
                                hardwareAccelerationH264.AppendProfile(FF_PROFILE_H264_CONSTRAINED_BASELINE);
                                hardwareAccelerationH264.AppendProfile(FF_PROFILE_H264_EXTENDED);
                                hardwareAccelerationH264.AppendProfile(FF_PROFILE_H264_MAIN);
                                hardwareAccelerationH264.AppendProfile(FF_PROFILE_H264_HIGH);
                            }
                            continue;
                        }

                        if (profile == D3D11_DECODER_PROFILE_HEVC_VLD_MAIN ||
                            profile == D3D11_DECODER_PROFILE_HEVC_VLD_MAIN10)
                        {
                            hardwareAccelerationHEVC.IsAvailable = true;

                            auto resolution = CheckResolution(profile, videoDevice);
                            hardwareAccelerationHEVC.MaxResolution = max(resolution, hardwareAccelerationHEVC.MaxResolution);

                            if (profile == D3D11_DECODER_PROFILE_HEVC_VLD_MAIN)
                            {
                                hardwareAccelerationHEVC.AppendProfile(FF_PROFILE_HEVC_MAIN, resolution);
                            }
                            else
                            {
                                hardwareAccelerationHEVC.AppendProfile(FF_PROFILE_HEVC_MAIN_10, resolution);
                            }
                            continue;
                        }

                        if (profile == D3D11_DECODER_PROFILE_MPEG2_VLD ||
                            profile == D3D11_DECODER_PROFILE_MPEG2and1_VLD)
                        {
                            if (!hardwareAccelerationMPEG2.IsAvailable)
                            {
                                hardwareAccelerationMPEG2.IsAvailable = true;
                                hardwareAccelerationMPEG2.AppendProfile(FF_PROFILE_MPEG2_MAIN);
                                hardwareAccelerationMPEG2.AppendProfile(FF_PROFILE_MPEG2_SIMPLE);
                            }

                            auto resolution = CheckResolution(profile, videoDevice);
                            hardwareAccelerationMPEG2.MaxResolution = max(resolution, hardwareAccelerationMPEG2.MaxResolution);
                            continue;
                        }

                        if (profile == D3D11_DECODER_PROFILE_VC1_VLD ||
                            profile == D3D11_DECODER_PROFILE_VC1_D2010)
                        {
                            auto resolution = CheckResolution(profile, videoDevice);

                            hardwareAccelerationVC1.IsAvailable = true;
                            hardwareAccelerationVC1.MaxResolution = max(resolution, hardwareAccelerationVC1.MaxResolution);

                            hardwareAccelerationWMV3.IsAvailable = true;
                            hardwareAccelerationWMV3.MaxResolution = hardwareAccelerationVC1.MaxResolution;
                            continue;
                        }

                        if (profile == D3D11_DECODER_PROFILE_VP9_VLD_PROFILE0 ||
                            profile == D3D11_DECODER_PROFILE_VP9_VLD_10BIT_PROFILE2)
                        {
                            hardwareAccelerationVP9.IsAvailable = true;

                            auto resolution = CheckResolution(profile, videoDevice);
                            hardwareAccelerationVP9.MaxResolution = max(resolution, hardwareAccelerationVP9.MaxResolution);

                            if (profile == D3D11_DECODER_PROFILE_VP9_VLD_PROFILE0)
                            {
                                hardwareAccelerationVP9.AppendProfile(FF_PROFILE_VP9_0, resolution);
                            }
                            else
                            {
                                hardwareAccelerationVP9.AppendProfile(FF_PROFILE_VP9_2, resolution);
                            }
                            continue;
                        }

                        if (profile == D3D11_DECODER_PROFILE_VP8_VLD)
                        {
                            hardwareAccelerationVP8.IsAvailable = true;

                            auto resolution = CheckResolution(profile, videoDevice);
                            hardwareAccelerationVP8.MaxResolution = resolution;
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

        static bool CheckUseHardwareAcceleration(HardwareAccelerationStatus status, AVCodecID codecId, int profile, int width, int height)
        {
            bool result = false;

            // auto detection
            if (status.IsAvailable)
            {
                // check profile, if restricted
                if (status.SupportedProfiles.size() > 0)
                {
                    for (auto& profileInfo : status.SupportedProfiles)
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
                    if (!result && !hasAskedInstallMpeg2Extension)
                    {
                        RaiseCodecRequired(CodecRequiredReason::HardwareAcceleration, L"MPEG2", L"MPEG2 Video Extension", L"9n95q1zzpmh4");
                        hasAskedInstallMpeg2Extension = true;
                    }
                }
                else if (codecId == AV_CODEC_ID_VP9 || codecId == AV_CODEC_ID_VP8)
                {
                    result &= CheckIsVP9VideoExtensionInstalled();
                    if (!result && !hasAskedInstallVP9Extension)
                    {
                        hstring codecName = codecId == AV_CODEC_ID_VP9 ? L"VP9" : L"VP8";
                        RaiseCodecRequired(CodecRequiredReason::HardwareAcceleration, codecName, L"VP9 Video Extensions", L"9n4d0msmp0pt");
                        hasAskedInstallVP9Extension = true;
                    }
                }
                else if (codecId == AV_CODEC_ID_HEVC)
                {
                    result &= CheckIsHEVCVideoExtensionInstalled();
                    if (!result && !hasAskedInstallHEVCExtension)
                    {
#ifdef _M_AMD64
                        // open free x64 extension
                        RaiseCodecRequired(CodecRequiredReason::HardwareAcceleration, L"HEVC", L"HEVC Video Extensions from Device Manufacturer", L"9n4wgh0z6vhq");
#else
                        // open paid extension for all other platforms
                        RaiseCodecRequired(CodecRequiredReason::HardwareAcceleration, L"HEVC", L"HEVC Video Extensions", L"9nmzlz57r3t7");
#endif // _WIN64
                        hasAskedInstallHEVCExtension = true;
                    }
                }

                CheckVideoResolution(result, width, height, status.MaxResolution);
            }

            return result;
        }

    private:
        static winrt::event<Windows::Foundation::EventHandler<winrt::FFmpegInteropX::CodecRequiredEventArgs>> m_codecRequiredEvent;
        static std::mutex mutex;

        static bool hasCheckedHardwareAcceleration;

        static bool hasCheckedMpeg2Extension;
        static bool hasCheckedVP9Extension;
        static bool hasCheckedHEVCExtension;

        static bool isMpeg2ExtensionInstalled;
        static bool isVP9ExtensionInstalled;
        static bool isHEVCExtensionInstalled;

        static bool hasAskedInstallMpeg2Extension;
        static bool hasAskedInstallVP9Extension;
        static bool hasAskedInstallHEVCExtension;

        static HardwareAccelerationStatus hardwareAccelerationH264;
        static HardwareAccelerationStatus hardwareAccelerationHEVC;
        static HardwareAccelerationStatus hardwareAccelerationWMV3;
        static HardwareAccelerationStatus hardwareAccelerationVC1;
        static HardwareAccelerationStatus hardwareAccelerationVP9;
        static HardwareAccelerationStatus hardwareAccelerationVP8;
        static HardwareAccelerationStatus hardwareAccelerationMPEG2;

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

        static IAsyncOperation<bool> IsAppInstalledAsync(hstring const& packageName)
        {
            using namespace winrt::Windows::System;
            try
            {
                auto t = co_await Launcher::QueryUriSupportAsync(Uri(L"mailto:dummy@mail.com"), LaunchQuerySupportType::Uri, packageName);
                switch (t)
                {
                case LaunchQuerySupportStatus::Available:
                case LaunchQuerySupportStatus::NotSupported:
                    co_return true;
                    //case LaunchQuerySupportStatus.AppNotInstalled:
                    //case LaunchQuerySupportStatus.AppUnavailable:
                    //case LaunchQuerySupportStatus.Unknown:
                default:
                    co_return false;
                }
            }
            catch (...)
            {
            }
            co_return false;
        }

        //// this works, but it takes 500ms on first call, so not using it right now...
        //static task<bool> IsVideoCodecInstalledAsync(hstring const& videoCodecSubtype)
        //{
        //	auto query =  CodecQuery();
        //	auto codecs = co_await query->FindAllAsync(CodecKind::Video, CodecCategory::Decoder, videoCodecSubtype);
        //	for (auto codec in codecs)
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
namespace winrt::FFmpegInteropX::factory_implementation
{
    struct CodecChecker : CodecCheckerT<CodecChecker, implementation::CodecChecker>
    {
    };
}
