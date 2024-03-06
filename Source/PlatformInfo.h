#pragma once

#include "pch.h"

#ifdef Win32
#include "winrt/Microsoft.UI.Xaml.h"
#endif

class PlatformInfo
{
private:
    static std::mutex guard;
    static bool hasCheckedXbox;
    static bool isXbox;

public:
    static bool IsXbox()
    {
        if (!hasCheckedXbox)
        {
            std::lock_guard lock(guard);
            if (!hasCheckedXbox)
            {
                try
                {
                    isXbox = winrt::Windows::System::Profile::AnalyticsInfo::VersionInfo().DeviceFamily() == L"Windows.Xbox";
                }
                catch (...)
                {
                }
                hasCheckedXbox = true;
            }
        }
        return isXbox;
    }

#ifdef Win32

private:
    static bool hasCheckedWinUI;
    static bool isWinUI;

public:
    static bool inline IsWinUI()
    {
        if (hasCheckedWinUI)
        {
            return isWinUI;
        }
        else
        {
            std::lock_guard lock(guard);
            if (!hasCheckedWinUI)
            {
                auto factory = winrt::try_get_activation_factory<winrt::Microsoft::UI::Xaml::FrameworkElement>();
                isWinUI = factory != nullptr;
                hasCheckedWinUI = true;
            }
            return isWinUI;
        }
    }

#else // UWP

public:
    static bool inline IsWinUI()
    {
        return false;
    }

#endif // WinUI
};
