#include "pch.h"
#include "FFmpegInteropLogging.h"
#include "FFmpegInteropLogging.g.cpp"

using namespace std;

namespace winrt::FFmpegInteropX::implementation
{
    ILogProvider FFmpegInteropLogging::s_pLogProvider = nullptr;
    mutex FFmpegInteropLogging::io_mutex;
    string FFmpegInteropLogging::line;

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
                    if (auto provider = s_pLogProvider)
                    {
                        char pLine[1000];
                        int printPrefix = 1;
                        av_log_format_line(avcl, level, fmt, vl, pLine, sizeof(pLine), &printPrefix);

                        {
                            lock_guard<mutex> lk(io_mutex);

                            // concatenate the result
                            line += pLine;

                            // send if the line ends with a new line
                            if (!line.empty() && line[line.size() - 1] == '\n')
                            {
                                provider.Log((LogLevel)level, StringUtils::Utf8ToPlatformString(line.c_str()));
                                line.clear();
                            }
                        }
                    }
                }
            });
    }

    void FFmpegInteropLogging::SetDefaultLogProvider()
    {
        av_log_set_callback(av_log_default_callback);
        s_pLogProvider = nullptr;
    }
}
