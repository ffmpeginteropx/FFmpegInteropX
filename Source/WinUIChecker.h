#pragma once

#ifdef WinUI

#include <mutex>
#include "winrt/Microsoft.UI.Xaml.h"

class WinUIChecker
{
public:
    static bool inline HasWinUI()
    {
        if (hasCheckedWinUI)
        {
            return hasWinUI;
        }
        else
        {
            auto lock = std::lock_guard(checkMutex);
            if (!hasCheckedWinUI)
            {
                auto factory = winrt::try_get_activation_factory<winrt::Microsoft::UI::Xaml::FrameworkElement>();
                hasWinUI = factory != nullptr;
                hasCheckedWinUI = true;
            }
            return hasWinUI;
        }
    }

private:
    static bool hasCheckedWinUI;
    static bool hasWinUI;
    static std::mutex checkMutex;
};

#endif
