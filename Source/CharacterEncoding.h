#pragma once
#include "CharacterEncoding.g.h"
#include <mutex>

namespace winrt::FFmpegInteropX::implementation
{
    struct CharacterEncoding : CharacterEncodingT<CharacterEncoding>
    {
        CharacterEncoding(int p_codePage, hstring const& p_name, hstring const& p_description);

        static Windows::Foundation::Collections::IVectorView<FFmpegInteropX::CharacterEncoding> AllEncodings();
        static FFmpegInteropX::CharacterEncoding SystemLocale();
        static FFmpegInteropX::CharacterEncoding ASCII();
        static FFmpegInteropX::CharacterEncoding UTF8();
        hstring Name();
        hstring Description();
        int32_t WindowsCodePage();

    private:
        hstring name{}, description{};
        int codePage = 0;
        static winrt::Windows::Foundation::Collections::IVector<winrt::FFmpegInteropX::CharacterEncoding> internalMap;
        static winrt::Windows::Foundation::Collections::IVectorView<winrt::FFmpegInteropX::CharacterEncoding> internalView;
        static std::mutex mutex;
    };
}
namespace winrt::FFmpegInteropX::factory_implementation
{
    struct CharacterEncoding : CharacterEncodingT<CharacterEncoding, implementation::CharacterEncoding>
    {
    };
}
