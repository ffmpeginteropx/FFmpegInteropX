#include "pch.h"
#include "FloatToDoubleConverter.h"

Platform::Object^ MediaPlayerCPP::FloatToDoubleConverter::Convert(Platform::Object^ value, Windows::UI::Xaml::Interop::TypeName targetType, Platform::Object^ parameter, Platform::String^ language)
{
	auto box = static_cast<Platform::IBox<float>^>(value);
	return ref new Platform::Box<double>(box->Value);
}

Platform::Object^ MediaPlayerCPP::FloatToDoubleConverter::ConvertBack(Platform::Object^ value, Windows::UI::Xaml::Interop::TypeName targetType, Platform::Object^ parameter, Platform::String^ language)
{
	auto box = static_cast<Platform::IBox<double>^>(value);
	return ref new Platform::Box<float>((float)box->Value);
}
