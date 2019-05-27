#include <string.h>
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

		static std::string* PlatformStringToChar(String^ value)
		{
			std::wstring strW(value->Begin());
			std::string* strA = new std::string(strW.begin(), strW.end());

			return strA;
		}	
	};
}