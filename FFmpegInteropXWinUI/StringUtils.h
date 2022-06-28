#pragma once
#include "pch.h"
#include <string.h>

namespace FFmpegInteropX
{
	class StringUtils
	{
	public:
		static winrt::hstring AnsiStringToPlatformString(const char * char_array)
		{
			if (!char_array) return winrt::hstring(L"");

			std::string s_str = std::string(char_array);
			std::wstring wid_str = std::wstring(s_str.begin(), s_str.end());
			const wchar_t* w_char = wid_str.c_str();
			return winrt::hstring(w_char);
		}

		static std::wstring Utf8ToWString(const char* char_array)
		{
			if (!char_array) return std::wstring(L"");

            auto required_size = MultiByteToWideChar(CP_UTF8, 0, char_array, -1, NULL, 0);
            wchar_t* buffer = (wchar_t*)calloc(required_size, sizeof(wchar_t));
            auto result = MultiByteToWideChar(CP_UTF8, 0, char_array, -1, buffer, required_size);
            assert(result == required_size);
            std::wstring wid_str = std::wstring(buffer);
            return wid_str;
		}

		static winrt::hstring Utf8ToPlatformString(const char* char_array)
		{
			auto wid_str = Utf8ToWString(char_array);
			return winrt::hstring(wid_str.c_str(), (int)wid_str.size());
		}

		static winrt::hstring WStringToPlatformString(const std::wstring& input)
		{
			return winrt::hstring(input.c_str(), (unsigned int)input.length());
		}


		static std::string PlatformStringToUtf8String(winrt::hstring value)
		{
			return to_string(value);
		}

	};
}
