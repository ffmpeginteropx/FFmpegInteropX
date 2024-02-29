#include "pch.h"

#ifdef Win32

#include "WinUIChecker.h"

bool WinUIChecker::hasCheckedWinUI;
bool WinUIChecker::hasWinUI;
std::mutex WinUIChecker::checkMutex;

#endif
