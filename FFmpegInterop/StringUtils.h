#include <string.h>
#include <codecvt>

using namespace Platform;

namespace FFmpegInterop
{
	ref class StringUtils sealed
	{
	internal:
		static String^ AnsiStringToPlatformString(const char * char_array) {
			std::string s_str = std::string(char_array);
			std::wstring wid_str = std::wstring(s_str.begin(), s_str.end());
			const wchar_t* w_char = wid_str.c_str();
			return ref new String(w_char);
		}

		static String^ Utf8ToPlatformString(const char* char_array) {
			std::string s_str = std::string(char_array);
			std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
			std::wstring wid_str = myconv.from_bytes(s_str);
			return ref new String(wid_str.c_str());
		}

	};
}