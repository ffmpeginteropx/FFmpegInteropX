#include "pch.h"
#include "MyEventArgs.h"
#include "MyEventArgs.g.cpp"

namespace winrt::FFmpegInteropX::implementation
{
    MyEventArgs::MyEventArgs(float temperatureFahrenheit) : m_temperatureFahrenheit(temperatureFahrenheit)
    {
    }

    float MyEventArgs::TemperatureFahrenheit()
    {
        return m_temperatureFahrenheit;
    }
    hstring MyEventArgs::Type()
    {
        return type;
    }
    int32_t MyEventArgs::Index()
    {
        return index;
    }
    int32_t MyEventArgs::StartTime()
    {
        return starttime;
    }
    int32_t MyEventArgs::Duration()
    {
        return duration;
    }
    int32_t MyEventArgs::Width()
    {
        return width;
    }
    int32_t MyEventArgs::Height()
    {
        return height;
    }
}
