#pragma once
#include "FFmpegInteropLogging.g.h"

namespace winrt::FFmpegInteropX::implementation
{
    struct FFmpegInteropLogging
    {
        static void SetLogLevel(FFmpegInteropX::LogLevel const& level);
        static void SetLogProvider(FFmpegInteropX::ILogProvider const& logProvider);
        static void SetDefaultLogProvider();

        static ILogProvider s_pLogProvider;
    };
}
namespace winrt::FFmpegInteropX::factory_implementation
{
    struct FFmpegInteropLogging : FFmpegInteropLoggingT<FFmpegInteropLogging, implementation::FFmpegInteropLogging>
    {
    };
}
