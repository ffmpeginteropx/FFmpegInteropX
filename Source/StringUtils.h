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
            return WStringToPlatformString(wid_str);
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
            return WStringToPlatformString(wid_str);
        }

        static String^ WStringToPlatformString(const std::wstring& input)
        {
            auto size = input.size();
            if (input[size - 1] == 0)
            {
                size--;
            }
            return ref new Platform::String(input.c_str(), (unsigned int)size);
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
