#pragma once

using namespace winrt::Windows::Foundation;

inline winrt::Windows::Foundation::TimeSpan operator*(const winrt::Windows::Foundation::TimeSpan& span, const double factor)
{
    auto mult = (long long)(span.count() * factor);
    return TimeSpan{ mult };
}

inline winrt::Windows::Foundation::TimeSpan operator/(const winrt::Windows::Foundation::TimeSpan& span, const double factor)
{
    auto mult = (long long)(span.count() / factor);
    return TimeSpan{ mult };
}
