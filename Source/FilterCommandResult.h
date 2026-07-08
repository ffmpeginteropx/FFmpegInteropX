#pragma once
#include "FilterCommandResult.g.h"

namespace winrt::FFmpegInteropX::implementation
{
    struct FilterCommandResult : FilterCommandResultT<FilterCommandResult>
    {
        FilterCommandResult(bool succeeded, hstring const& result)
        {
            Succeeded(succeeded);
            Result(result);
        }

        PROPERTY_CONST(Succeeded, bool, false);

        PROPERTY_CONST(Result, hstring, hstring{});
    };
}
namespace winrt::FFmpegInteropX::factory_implementation
{
    struct FilterCommandResult : FilterCommandResultT<FilterCommandResult, implementation::FilterCommandResult>
    {
    };
}
