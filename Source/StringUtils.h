#pragma once
#include "pch.h"
#include <string.h>

namespace FFmpegInteropX
{
    class StringUtils
    {
    public:
        static winrt::hstring AnsiStringToPlatformString(const char* char_array)
        {
            if (!char_array) return winrt::hstring(L"");

            std::string s_str = std::string(char_array);
            std::wstring wid_str = std::wstring(s_str.begin(), s_str.end());
            return WStringToPlatformString(wid_str);
        }

        static std::wstring Utf8ToWString(const char* char_array)
        {
            if (!char_array) return std::wstring(L"");

            auto required_size = MultiByteToWideChar(CP_UTF8, 0, char_array, -1, NULL, 0);
            std::wstring wid_str = std::wstring(required_size, '?');
            auto result = MultiByteToWideChar(CP_UTF8, 0, char_array, -1, wid_str.data(), required_size);
            assert(result == required_size);
            return wid_str;
        }

        static winrt::hstring Utf8ToPlatformString(const char* char_array)
        {
            auto wid_str = Utf8ToWString(char_array);
            return WStringToPlatformString(wid_str);
        }

        static winrt::hstring WStringToPlatformString(const std::wstring& input)
        {
            auto size = input.size();
            auto sizeWithotTrailingZero = input[size - 1] == 0 ? size - 1 : size;
            return winrt::hstring(input.c_str(), sizeWithotTrailingZero);
        }

        static std::string PlatformStringToUtf8String(winrt::hstring value)
        {
            return to_string(value);
        }

    };
}
