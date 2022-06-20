#pragma once
#include "FFmpegInteropLogging.g.h"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropXWinUI::implementation
{
	struct FFmpegInteropLogging : FFmpegInteropLoggingT<FFmpegInteropLogging>
	{
		FFmpegInteropLogging() = default;

		static void SetLogLevel(FFmpegInteropXWinUI::LogLevel const& level);
		static void SetLogProvider(FFmpegInteropXWinUI::ILogProvider const& logProvider);
		static void SetDefaultLogProvider();

		static ILogProvider s_pLogProvider;
	};
}
namespace winrt::FFmpegInteropXWinUI::factory_implementation
{
	struct FFmpegInteropLogging : FFmpegInteropLoggingT<FFmpegInteropLogging, implementation::FFmpegInteropLogging>
	{
	};
}
