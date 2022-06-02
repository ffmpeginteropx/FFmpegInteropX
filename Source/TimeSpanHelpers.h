#pragma once

using namespace winrt::Windows::Foundation;

inline bool operator<(const winrt::Windows::Foundation::TimeSpan& span, const winrt::Windows::Foundation::TimeSpan& other)
{
	return span < other;
}
inline bool operator<=(const winrt::Windows::Foundation::TimeSpan& span, const winrt::Windows::Foundation::TimeSpan& other)
{
	return span <= other;
}
inline bool operator>(const winrt::Windows::Foundation::TimeSpan& span, const winrt::Windows::Foundation::TimeSpan& other)
{
	return span > other;
}
inline bool operator>=(const winrt::Windows::Foundation::TimeSpan& span, const winrt::Windows::Foundation::TimeSpan& other)
{
	return span >= other;
}
inline bool operator==(const winrt::Windows::Foundation::TimeSpan& span, const winrt::Windows::Foundation::TimeSpan& other)
{
	return span == other;
}
inline bool operator!=(const winrt::Windows::Foundation::TimeSpan& span, const winrt::Windows::Foundation::TimeSpan& other)
{
	return span != other;
}

inline winrt::Windows::Foundation::TimeSpan operator+(const winrt::Windows::Foundation::TimeSpan& span, const winrt::Windows::Foundation::TimeSpan& other)
{
	winrt::Windows::Foundation::TimeSpan result;
	result = span + other;
	return result;
}

inline winrt::Windows::Foundation::TimeSpan operator-(const winrt::Windows::Foundation::TimeSpan& span, const winrt::Windows::Foundation::TimeSpan& other)
{
	winrt::Windows::Foundation::TimeSpan result;
	result = span - other;
	return result;
}

inline winrt::Windows::Foundation::TimeSpan& operator+=(const winrt::Windows::Foundation::TimeSpan& span, const winrt::Windows::Foundation::TimeSpan& other)
{
	return span += other;
}

inline winrt::Windows::Foundation::TimeSpan& operator-=(const winrt::Windows::Foundation::TimeSpan& span, const winrt::Windows::Foundation::TimeSpan& other)
{
	return span -= other;
}

inline winrt::Windows::Foundation::TimeSpan operator*(const winrt::Windows::Foundation::TimeSpan& span, const double factor)
{
	return (span * factor);
}

inline winrt::Windows::Foundation::TimeSpan operator/(const winrt::Windows::Foundation::TimeSpan& span, const double factor)
{
	return (span / factor);
}

//inline winrt::Windows::Foundation::TimeSpan TowinrtTimeSpan(long long ticks)
//{
//	return std::chrono::duration<long, long> a(ticks)
//}