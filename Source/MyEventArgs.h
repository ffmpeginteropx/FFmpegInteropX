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
        winrt::Windows::Storage::Streams::IBuffer Buffer();
        winrt::Windows::Storage::Streams::IBuffer Buffer2();

    private:
        float m_temperatureFahrenheit{ 0.f };
        hstring type{};
        int32_t index{};
        int32_t starttime{};
        int32_t duration{};
        int32_t width{};
        int32_t height{};
        winrt::Windows::Storage::Streams::IBuffer buffer{};
        winrt::Windows::Storage::Streams::IBuffer buffer2{};
    };
}
