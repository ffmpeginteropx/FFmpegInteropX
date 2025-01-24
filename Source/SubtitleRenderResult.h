#pragma once
#include "SubtitleRenderResult.g.h"

namespace winrt::FFmpegInteropX::implementation
{
    struct SubtitleRenderResult : SubtitleRenderResultT<SubtitleRenderResult>
    {
        SubtitleRenderResult()
        {
            Succeeded(false);
            HasChanged(false);
        }

        SubtitleRenderResult(bool succeeded, bool hasChanged)
        {
            Succeeded(succeeded);
            HasChanged(hasChanged);
        }

        PROPERTY_CONST(Succeeded, bool, false);
        PROPERTY_CONST(HasChanged, bool, false);
    };
}
