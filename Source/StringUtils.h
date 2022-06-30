#include <string.h>
#include <codecvt>

namespace FFmpegInteropX
{
    using namespace Platform;

    ref class StringUtils sealed
    {
    internal:
        static String^ AnsiStringToPlatformString(const char* char_array)
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

            auto required_size = MultiByteToWideChar(CP_UTF8, 0, char_array, -1, NULL, 0);
            wchar_t* buffer = (wchar_t*)calloc(required_size, sizeof(wchar_t));
            auto result = MultiByteToWideChar(CP_UTF8, 0, char_array, -1, buffer, required_size);
            std::wstring wid_str = std::wstring(buffer);
            free(buffer);
            return wid_str;
        }

        static String^ Utf8ToPlatformString(const char* char_array)
        {
            auto wid_str = Utf8ToWString(char_array);
            return ref new String(wid_str.c_str(), (int)wid_str.size());
        }

        static String^ WStringToPlatformString(const std::wstring& input)
        {
            return ref new Platform::String(input.c_str(), (unsigned int)input.length());
        }

        static std::string PlatformStringToUtf8String(String^ value)
        {
            int required_size = WideCharToMultiByte(CP_UTF8, 0, value->Data(), -1, NULL, 0, NULL, NULL);
            char* buffer = (char*)calloc(required_size, sizeof(char));
            auto result = WideCharToMultiByte(CP_UTF8, 0, value->Data(), -1, buffer, required_size, NULL, NULL);
            std::string s_str = std::string(buffer);
            free(buffer);
            return s_str;
        }

    };
}
