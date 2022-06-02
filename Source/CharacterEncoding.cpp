#include "pch.h"
#include "CharacterEncoding.h"
#if __has_include("CharacterEncoding.g.cpp")
#include "CharacterEncoding.g.cpp"
#endif

namespace winrt::FFmpegInteropX::implementation 
{
	winrt::Windows::Foundation::Collections::IVector<CharacterEncoding> FFmpegInteropX::CharacterEncoding::internalMap;
	winrt::Windows::Foundation::Collections::IVectorView<CharacterEncoding> FFmpegInteropX::CharacterEncoding::internalView;
	static std::mutex FFmpegInteropX::CharacterEncoding::mutex;
}