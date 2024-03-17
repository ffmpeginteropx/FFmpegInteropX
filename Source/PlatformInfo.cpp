#include "pch.h"
#include "PlatformInfo.h"

std::mutex PlatformInfo::guard;
bool PlatformInfo::hasCheckedXbox = false;
bool PlatformInfo::isXbox = false;

#ifdef Win32

bool PlatformInfo::hasCheckedWinUI = false;
bool PlatformInfo::isWinUI = false;

#endif // Win32
