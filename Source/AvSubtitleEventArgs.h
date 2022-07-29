#pragma once
#include "AvSubtitleEventArgs.g.h"

namespace winrt::FFmpegInteropX::implementation
{
    struct AvSubtitleEventArgs : AvSubtitleEventArgsT<AvSubtitleEventArgs>
    {
        AvSubtitleEventArgs::AvSubtitleEventArgs(Windows::Foundation::TimeSpan const& startTime, Windows::Foundation::TimeSpan const& duration);
        AvSubtitleEventArgs::AvSubtitleEventArgs();
        hstring Type();
        int32_t Index();
        Windows::Foundation::TimeSpan StartTime();
        Windows::Foundation::TimeSpan Duration();
        int32_t Width();
        int32_t Height();
        winrt::Windows::Storage::Streams::IBuffer Buffer();
        winrt::Windows::Storage::Streams::IBuffer Buffer2();

    private:
        hstring type{};
        int32_t index{};
        Windows::Foundation::TimeSpan starttime{};
        Windows::Foundation::TimeSpan duration{};
        int32_t width{};
        int32_t height{};
        winrt::Windows::Storage::Streams::IBuffer buffer{};
        winrt::Windows::Storage::Streams::IBuffer buffer2{};
    };
}
namespace winrt::FFmpegInteropX::factory_implementation
{
    struct AvSubtitleEventArgs : AvSubtitleEventArgsT<AvSubtitleEventArgs, implementation::AvSubtitleEventArgs>
    {
    };
}
