#include "pch.h"
#include "PlatformInfo.h"

std::mutex PlatformInfo::guard;
bool PlatformInfo::hasChecked = false;
bool PlatformInfo::isXbox = false;
