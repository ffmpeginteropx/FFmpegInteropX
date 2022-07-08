#include <string.h>
#include <codecvt>

namespace FFmpegInteropX
{
    using namespace Platform;

    ref class StringUtils sealed
    {
    internal:
        static std::wstring Utf8ToWString(const char* char_array)
        {
            auto required_size = MultiByteToWideChar(CP_UTF8, 0, char_array, -1, NULL, 0);
            if (required_size > 0)
            {
                std::wstring wid_str = std::wstring(required_size - 1, '?');
                auto result = MultiByteToWideChar(CP_UTF8, 0, char_array, -1, (LPWSTR)wid_str.data(), required_size);
                return wid_str;
            }
            return std::wstring(L"");
        }

        static String^ Utf8ToPlatformString(const char* char_array)
        {
            auto wid_str = Utf8ToWString(char_array);
            return WStringToPlatformString(wid_str);
        }

        static String^ WStringToPlatformString(const std::wstring& input)
        {
            return ref new Platform::String(input.c_str(), (unsigned int)input.size());
        }

        static std::string PlatformStringToUtf8String(String^ value)
        {
            int required_size = WideCharToMultiByte(CP_UTF8, 0, value->Data(), -1, NULL, 0, NULL, NULL);
            if (required_size > 0)
            {
                std::string s_str = std::string(required_size - 1, '?');
                auto result = WideCharToMultiByte(CP_UTF8, 0, value->Data(), -1, (LPSTR)s_str.data(), required_size, NULL, NULL);
                return s_str;
            }
            return std::string("");
        }

    };
}
