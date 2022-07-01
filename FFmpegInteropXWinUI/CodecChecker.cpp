#include "pch.h"
#include "CodecChecker.h"
#include "CodecChecker.g.cpp"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropXWinUI::implementation
{
    winrt::event_token CodecChecker::CodecRequired(winrt::Windows::Foundation::EventHandler<winrt::FFmpegInteropXWinUI::CodecRequiredEventArgs> const& handler)
    {
        return m_codecRequiredEvent.add(handler);
    }

    void CodecChecker::CodecRequired(winrt::event_token const& token) noexcept
    {
        m_codecRequiredEvent.remove(token);
    }

    Windows::Foundation::IAsyncAction CodecChecker::InitializeAsync()
    {
        co_await concurrency::create_task(&Initialize);
    }

    Windows::Foundation::IAsyncAction CodecChecker::RefreshAsync()
    {
        co_await concurrency::create_task(&Refresh);
    }

    Windows::Foundation::IAsyncOperation<bool> CodecChecker::CheckIsMpeg2VideoExtensionInstalledAsync()
    {
        hasCheckedMpeg2Extension = false;
        auto rvalue = co_await concurrency::create_task(&CheckIsMpeg2VideoExtensionInstalled);
        co_return rvalue;
    }

    Windows::Foundation::IAsyncOperation<bool> CodecChecker::CheckIsVP9VideoExtensionInstalledAsync()
    {
        hasCheckedVP9Extension = false;
        auto rvalue = co_await concurrency::create_task(&CheckIsVP9VideoExtensionInstalled);
        co_return rvalue;
    }

    Windows::Foundation::IAsyncOperation<bool> CodecChecker::CheckIsHEVCVideoExtensionInstalledAsync()
    {
        hasCheckedHEVCExtension = false;
        auto rvalue = co_await concurrency::create_task(&CheckIsHEVCVideoExtensionInstalled);
        co_return rvalue;
    }

    Windows::Foundation::IAsyncOperation<bool> CodecChecker::OpenMpeg2VideoExtensionStoreEntryAsync()
    {
        hasCheckedMpeg2Extension = false;
        return winrt::Windows::System::Launcher::LaunchUriAsync(Uri(L"ms-windows-store://pdp/?ProductId=9n95q1zzpmh4"));
    }

    Windows::Foundation::IAsyncOperation<bool> CodecChecker::OpenVP9VideoExtensionStoreEntryAsync()
    {
        hasCheckedVP9Extension = false;
        return winrt::Windows::System::Launcher::LaunchUriAsync(Uri(L"ms-windows-store://pdp/?ProductId=9n4d0msmp0pt"));
    }

    Windows::Foundation::IAsyncOperation<bool> CodecChecker::OpenHEVCVideoExtensionStoreEntryAsync()
    {
        hasCheckedHEVCExtension = false;
#ifdef _M_AMD64
        // open free x64 extension
        return winrt::Windows::System::Launcher::LaunchUriAsync(Uri(L"ms-windows-store://pdp/?ProductId=9n4wgh0z6vhq"));
#else
        // open paid extension for all other platforms
        return winrt::Windows::System::Launcher::LaunchUriAsync(Uri(L"ms-windows-store://pdp/?ProductId=9nmzlz57r3t7"));
#endif // _WIN64

    }

    winrt::event<Windows::Foundation::EventHandler<winrt::FFmpegInteropXWinUI::CodecRequiredEventArgs>> CodecChecker::m_codecRequiredEvent {};
    std::mutex CodecChecker::mutex;

    bool CodecChecker::hasCheckedHardwareAcceleration = false;

    bool CodecChecker::hasCheckedMpeg2Extension = false;
    bool CodecChecker::hasCheckedVP9Extension = false;
    bool CodecChecker::hasCheckedHEVCExtension = false;

    bool CodecChecker::isMpeg2ExtensionInstalled = false;
    bool CodecChecker::isVP9ExtensionInstalled = false;
    bool CodecChecker::isHEVCExtensionInstalled = false;

    bool CodecChecker::hasAskedInstallMpeg2Extension = false;
    bool CodecChecker::hasAskedInstallVP9Extension = false;
    bool CodecChecker::hasAskedInstallHEVCExtension = false;

    HardwareAccelerationStatus CodecChecker::hardwareAccelerationH264;
    HardwareAccelerationStatus CodecChecker::hardwareAccelerationHEVC;
    HardwareAccelerationStatus CodecChecker::hardwareAccelerationWMV3;
    HardwareAccelerationStatus CodecChecker::hardwareAccelerationVC1;
    HardwareAccelerationStatus CodecChecker::hardwareAccelerationVP9;
    HardwareAccelerationStatus CodecChecker::hardwareAccelerationVP8;
    HardwareAccelerationStatus CodecChecker::hardwareAccelerationMPEG2;
}
