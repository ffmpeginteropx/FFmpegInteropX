#include "pch.h"
#include "CharacterEncoding.h"
#include "CharacterEncoding.g.cpp"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropXWinUI::implementation
{
    Windows::Foundation::Collections::IVectorView<FFmpegInteropXWinUI::CharacterEncoding> CharacterEncoding::GetCharacterEncodings()
    {
        throw hresult_not_implemented();
    }
    FFmpegInteropXWinUI::CharacterEncoding CharacterEncoding::GetSystemDefault()
    {
        throw hresult_not_implemented();
    }
    hstring CharacterEncoding::Name()
    {
        throw hresult_not_implemented();
    }
    hstring CharacterEncoding::Description()
    {
        throw hresult_not_implemented();
    }
    int32_t CharacterEncoding::WindowsCodePage()
    {
        throw hresult_not_implemented();
    }
}
