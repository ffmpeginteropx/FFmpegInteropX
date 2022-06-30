#pragma once


namespace MediaPlayerCPP
{
    using namespace Windows::Foundation;
    using namespace Windows::UI::Xaml::Data;

    public ref class TimeSpanToDoubleConverter sealed : public IValueConverter
    {
    public:

        // Inherited via IValueConverter
        virtual Platform::Object^ Convert(Platform::Object^ value, Windows::UI::Xaml::Interop::TypeName targetType, Platform::Object^ parameter, Platform::String^ language)
        {
            auto box = static_cast<Platform::IBox<TimeSpan>^>(value);
            return ref new Platform::Box<double>((double)box->Value.Duration / 10000000);
        }

        virtual Platform::Object^ ConvertBack(Platform::Object^ value, Windows::UI::Xaml::Interop::TypeName targetType, Platform::Object^ parameter, Platform::String^ language)
        {
            auto box = static_cast<Platform::IBox<double>^>(value);
            return ref new Platform::Box<TimeSpan>(TimeSpan{ (long long)(box->Value * 10000000) });
        }
    };
}