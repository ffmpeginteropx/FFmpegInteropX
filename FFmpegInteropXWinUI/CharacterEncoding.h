#pragma once
#include "CharacterEncoding.g.h"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropXWinUI::implementation
{
    struct CharacterEncoding : CharacterEncodingT<CharacterEncoding>
    {
        CharacterEncoding() = default;

        static Windows::Foundation::Collections::IVectorView<FFmpegInteropXWinUI::CharacterEncoding> GetCharacterEncodings();
        static FFmpegInteropXWinUI::CharacterEncoding GetSystemDefault();
        hstring Name();
        hstring Description();
        int32_t WindowsCodePage();
    };
}
namespace winrt::FFmpegInteropXWinUI::factory_implementation
{
    struct CharacterEncoding : CharacterEncodingT<CharacterEncoding, implementation::CharacterEncoding>
    {
    };
}
