#include "pch.h"
#include <string.h>
#include <codecvt>
#define  _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS

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

			std::string s_str = std::string(char_array);
			std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
			std::wstring wid_str = myconv.from_bytes(s_str);
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
			/*std::wstring strW(value->Begin(), value->Length());
			std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
			return myconv.to_bytes(strW);*/
		}

	};
}