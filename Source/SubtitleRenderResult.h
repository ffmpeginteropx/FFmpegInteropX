#pragma once
#include "SubtitleRenderResult.g.h"

namespace winrt::FFmpegInteropX::implementation
{
    struct SubtitleRenderResult : SubtitleRenderResultT<SubtitleRenderResult>
    {
        SubtitleRenderResult(bool succeeded)
        {
            Succeeded(succeeded);
        }

        PROPERTY_CONST(Succeeded, bool, false);
    };
}
