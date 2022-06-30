#pragma once


namespace MediaPlayerCPP
{
    using namespace Windows::UI::Xaml::Data;

    public ref class FloatToDoubleConverter sealed : public IValueConverter
    {
    public:

        // Inherited via IValueConverter
        virtual Platform::Object^ Convert(Platform::Object^ value, Windows::UI::Xaml::Interop::TypeName targetType, Platform::Object^ parameter, Platform::String^ language);
        virtual Platform::Object^ ConvertBack(Platform::Object^ value, Windows::UI::Xaml::Interop::TypeName targetType, Platform::Object^ parameter, Platform::String^ language);
    };
}