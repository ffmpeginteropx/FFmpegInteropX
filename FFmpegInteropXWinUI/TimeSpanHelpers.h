#pragma once

using namespace winrt::Windows::Foundation;

inline bool operator < (const winrt::Windows::Foundation::TimeSpan& span, const winrt::Windows::Foundation::TimeSpan& other)
{
	return span.count() < other.count();
}
inline bool operator <= (const winrt::Windows::Foundation::TimeSpan& span, const winrt::Windows::Foundation::TimeSpan& other)
{
	return span.count() <= other.count();
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
	winrt::Windows::Foundation::TimeSpan result(span.count() + other.count());
	return result;
}

inline winrt::Windows::Foundation::TimeSpan operator-(const winrt::Windows::Foundation::TimeSpan& span, const winrt::Windows::Foundation::TimeSpan& other)
{
	winrt::Windows::Foundation::TimeSpan result(span.count() - other.count());
	return result;
}

inline winrt::Windows::Foundation::TimeSpan& operator += (const winrt::Windows::Foundation::TimeSpan& span, const winrt::Windows::Foundation::TimeSpan& other)
{
	return winrt::Windows::Foundation::TimeSpan(span.count() + other.count());
}

inline winrt::Windows::Foundation::TimeSpan& operator-=(const winrt::Windows::Foundation::TimeSpan& span, const winrt::Windows::Foundation::TimeSpan& other)
{
	return winrt::Windows::Foundation::TimeSpan(span.count() - other.count());
}

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

//inline winrt::Windows::Foundation::TimeSpan TowinrtTimeSpan(long long ticks)
//{
//	return std::chrono::duration<long, long> a(ticks)
//}