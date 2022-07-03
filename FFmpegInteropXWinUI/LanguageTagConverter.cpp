#include "pch.h"
#include "LanguageTagConverter.h"

std::map<winrt::hstring, std::shared_ptr<FFmpegInteropX::LanguageEntry>> FFmpegInteropX::LanguageTagConverter::map;