#include "pch.h"

#ifdef WinUI

#include "WinUIChecker.h"

bool WinUIChecker::hasCheckedWinUI;
bool WinUIChecker::hasWinUI;
std::mutex WinUIChecker::checkMutex;

#endif
