#include "pch.h"
#include "CodecChecker.h"

using namespace FFmpegInterop;

std::mutex CodecChecker::mutex;

bool CodecChecker::hasCheckedHardwareAcceleration = false;
bool CodecChecker::hasCheckedMpeg2Extension = false;
bool CodecChecker::hasCheckedVP9Extension = false;

bool CodecChecker::isMpeg2ExtensionInstalled = false;
bool CodecChecker::isVP9ExtensionInstalled = false;

HardwareAccelerationStatus^ CodecChecker::hardwareAccelerationH264 = ref new HardwareAccelerationStatus();
HardwareAccelerationStatus^ CodecChecker::hardwareAccelerationHEVC = ref new HardwareAccelerationStatus();
HardwareAccelerationStatus^ CodecChecker::hardwareAccelerationWMV3 = ref new HardwareAccelerationStatus();
HardwareAccelerationStatus^ CodecChecker::hardwareAccelerationVC1 = ref new HardwareAccelerationStatus();
HardwareAccelerationStatus^ CodecChecker::hardwareAccelerationVP9 = ref new HardwareAccelerationStatus();
HardwareAccelerationStatus^ CodecChecker::hardwareAccelerationMPEG2 = ref new HardwareAccelerationStatus();