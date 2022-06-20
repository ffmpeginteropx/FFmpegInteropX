#include "pch.h"
#include "CodecChecker.h"

using namespace FFmpegInteropX;

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

HardwareAccelerationStatus^ CodecChecker::hardwareAccelerationH264 = ref new HardwareAccelerationStatus();
HardwareAccelerationStatus^ CodecChecker::hardwareAccelerationHEVC = ref new HardwareAccelerationStatus();
HardwareAccelerationStatus^ CodecChecker::hardwareAccelerationWMV3 = ref new HardwareAccelerationStatus();
HardwareAccelerationStatus^ CodecChecker::hardwareAccelerationVC1 = ref new HardwareAccelerationStatus();
HardwareAccelerationStatus^ CodecChecker::hardwareAccelerationVP9 = ref new HardwareAccelerationStatus();
HardwareAccelerationStatus^ CodecChecker::hardwareAccelerationVP8 = ref new HardwareAccelerationStatus();
HardwareAccelerationStatus^ CodecChecker::hardwareAccelerationMPEG2 = ref new HardwareAccelerationStatus();