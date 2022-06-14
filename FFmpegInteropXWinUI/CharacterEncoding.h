#pragma once
#include "CharacterEncoding.g.h"
#include <mutex>

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropXWinUI::implementation
{
    struct CharacterEncoding : CharacterEncodingT<CharacterEncoding>
    {
        CharacterEncoding();
        CharacterEncoding(int p_codePage, hstring const& p_name, hstring const& p_description);

        static Windows::Foundation::Collections::IVectorView<FFmpegInteropXWinUI::CharacterEncoding> GetCharacterEncodings();
        static FFmpegInteropXWinUI::CharacterEncoding GetSystemDefault();
        hstring Name();
        hstring Description();
        int32_t WindowsCodePage();

    private:
        hstring name, description;
        int codePage;
        static winrt::Windows::Foundation::Collections::IVector<winrt::FFmpegInteropXWinUI::CharacterEncoding> internalMap;
        static winrt::Windows::Foundation::Collections::IVectorView<winrt::FFmpegInteropXWinUI::CharacterEncoding> internalView;
        static std::mutex mutex;
    };
}
namespace winrt::FFmpegInteropXWinUI::factory_implementation
{
    struct CharacterEncoding : CharacterEncodingT<CharacterEncoding, implementation::CharacterEncoding>
    {
    };
}
