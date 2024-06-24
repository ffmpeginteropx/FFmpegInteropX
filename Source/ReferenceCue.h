#pragma once

struct ReferenceCue : winrt::implements<ReferenceCue, winrt::Windows::Media::Core::IMediaCue>
{
    ReferenceCue(winrt::Windows::Media::Core::IMediaCue const& addedCue, winrt::Windows::Media::Core::IMediaCue const& changedCue);
    ReferenceCue(winrt::Windows::Foundation::TimeSpan const& startTime, winrt::Windows::Foundation::TimeSpan const& duration, winrt::Windows::Media::Core::IMediaCue const& changedCue);
    void StartTime(winrt::Windows::Foundation::TimeSpan const& value);
    winrt::Windows::Foundation::TimeSpan StartTime();
    void Duration(winrt::Windows::Foundation::TimeSpan const& value);
    winrt::Windows::Foundation::TimeSpan Duration();
    void Id(winrt::hstring const& value);
    winrt::hstring Id();
    winrt::Windows::Media::Core::IMediaCue AddedCue();
    winrt::Windows::Media::Core::IMediaCue ChangedCue();

private:
    winrt::hstring id{};
    winrt::Windows::Foundation::TimeSpan duration{};
    winrt::Windows::Foundation::TimeSpan startTime{};
    winrt::Windows::Media::Core::IMediaCue addedCue = { nullptr };
    winrt::Windows::Media::Core::IMediaCue changedCue = { nullptr };
};
