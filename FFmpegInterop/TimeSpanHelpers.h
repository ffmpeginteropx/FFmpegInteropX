#pragma once

using namespace Windows::Foundation;

inline bool operator<(const TimeSpan& span, const TimeSpan& other)
{
	return span.Duration < other.Duration;
}
inline bool operator<=(const TimeSpan& span, const TimeSpan& other)
{
	return span.Duration <= other.Duration;
}
inline bool operator>(const TimeSpan& span, const TimeSpan& other)
{
	return span.Duration > other.Duration;
}
inline bool operator>=(const TimeSpan& span, const TimeSpan& other)
{
	return span.Duration >= other.Duration;
}
inline bool operator==(const TimeSpan& span, const TimeSpan& other)
{
	return span.Duration == other.Duration;
}

inline TimeSpan operator+(const TimeSpan& span, const TimeSpan& other)
{
	TimeSpan result;
	result.Duration = span.Duration + other.Duration;
	return result;
}

inline TimeSpan operator-(const TimeSpan& span, const TimeSpan& other)
{
	TimeSpan result;
	result.Duration = span.Duration - other.Duration;
	return result;
}

inline TimeSpan& operator+=(TimeSpan& span, const TimeSpan& other)
{
	span.Duration += other.Duration;
	return span;
}

inline TimeSpan& operator-=(TimeSpan& span, const TimeSpan& other)
{
	span.Duration -= other.Duration;
	return span;
}

inline TimeSpan ToTimeSpan(long long ticks)
{
	TimeSpan result;
	result.Duration = ticks;
	return result;
}