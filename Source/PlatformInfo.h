#pragma once

#include "pch.h"

class PlatformInfo
{
private:
    static std::mutex guard;
    static bool hasChecked;
    static bool isXbox;

public:
    static bool IsXbox()
    {
        if (!hasChecked)
        {
            std::lock_guard lock(guard);
            if (!hasChecked)
            {
                try
                {
                    isXbox = winrt::Windows::System::Profile::AnalyticsInfo::VersionInfo().DeviceFamily() == L"Windows.Xbox";
                }
                catch (...)
                {
                }
                hasChecked = true;
            }
        }
        return isXbox;
    }
};
