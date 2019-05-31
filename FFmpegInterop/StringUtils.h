#include <string.h>
#include <codecvt>

using namespace Platform;

namespace FFmpegInterop
{
	ref class StringUtils sealed
	{
	internal:
		static String^ AnsiStringToPlatformString(const char * char_array)
		{
			if (!char_array) return "";

			std::string s_str = std::string(char_array);
			std::wstring wid_str = std::wstring(s_str.begin(), s_str.end());
			const wchar_t* w_char = wid_str.c_str();
			return ref new String(w_char);
		}

		static std::wstring Utf8ToWString(const char* char_array)
		{
			if (!char_array) return std::wstring(L"");

			std::string s_str = std::string(char_array);
			std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
			std::wstring wid_str = myconv.from_bytes(s_str);
			return wid_str;
		}

		static String^ Utf8ToPlatformString(const char* char_array)
		{
			auto wid_str = Utf8ToWString(char_array);
			return ref new String(wid_str.c_str(), wid_str.size());
		}

		static String^ WStringToPlatformString(const std::wstring& input)
		{
			return ref new Platform::String(input.c_str(), (unsigned int)input.length());
		}


		static std::string PlatformStringToAnsiString(String^ value)
		{
			std::wstring strW(value->Begin());
			std::string strA = std::string(strW.begin(), strW.end());

			return strA;
		}

	};
}