#pragma once
#include "MyEventArgs.g.h"

namespace winrt::FFmpegInteropX::implementation
{
    struct MyEventArgs : MyEventArgsT<MyEventArgs>
    {
        MyEventArgs() = default;
        MyEventArgs(float temperatureFahrenheit);
        float TemperatureFahrenheit();
        hstring Type();
        int32_t Index();
        int32_t StartTime();
        int32_t Duration();
        int32_t Width();
        int32_t Height();

    private:
        float m_temperatureFahrenheit{ 0.f };
        hstring type{};
        int32_t index{};
        int32_t starttime{};
        int32_t duration{};
        int32_t width{};
        int32_t height{};
    };
}
