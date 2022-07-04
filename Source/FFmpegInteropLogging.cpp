#include "pch.h"
#include "FFmpegInteropLogging.h"
#include "FFmpegInteropLogging.g.cpp"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropX::implementation
{
    ILogProvider FFmpegInteropLogging::s_pLogProvider = nullptr;

    void FFmpegInteropLogging::SetLogLevel(FFmpegInteropX::LogLevel const& level)
    {
        av_log_set_level((int)level);
    }

    void FFmpegInteropLogging::SetLogProvider(FFmpegInteropX::ILogProvider const& logProvider)
    {
        s_pLogProvider = logProvider;

        av_log_set_callback([](void* avcl, int level, const char* fmt, va_list vl)->void
            {
                if (level <= av_log_get_level())
                {
                    if (s_pLogProvider != nullptr)
                    {
                        char pLine[1000];
                        int printPrefix = 1;
                        av_log_format_line(avcl, level, fmt, vl, pLine, sizeof(pLine), &printPrefix);

                        wchar_t wLine[sizeof(pLine)];
                        if (MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pLine, -1, wLine, sizeof(pLine)) != 0)
                        {
                            s_pLogProvider.Log((LogLevel)level, hstring(wLine));
                        }
                    }
                }
            });
    }

    void FFmpegInteropLogging::SetDefaultLogProvider()
    {
        av_log_set_callback(av_log_default_callback);
    }
}
