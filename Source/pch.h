#pragma once
#include <unknwn.h>
#include <shcore.h>
#include <memory>

// prevent compiler warnings due to name conflicts
#pragma push_macro("GetCurrentTime")
#pragma push_macro("TRY")
#undef GetCurrentTime
#undef TRY
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Graphics.Display.h>
#include <winrt/Windows.Graphics.Display.Core.h>
#include <winrt/Windows.Graphics.DirectX.Direct3D11.h>
#include <winrt/Windows.Graphics.DirectX.h>
#include <winrt/Windows.Graphics.Imaging.h>
#include <winrt/Windows.Globalization.h>
#include <winrt/Windows.Foundation.Metadata.h>
#include <winrt/Windows.Media.h>
#include <winrt/Windows.Media.Core.h>
#include <winrt/Windows.Media.MediaProperties.h>
#include <winrt/Windows.Media.Playback.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Core.Preview.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.FileProperties.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.Media.Core.h>
#pragma pop_macro("TRY")
#pragma pop_macro("GetCurrentTime")

#include <d3d11.h>
#include <windows.graphics.directx.direct3d11.interop.h>

#include <mfapi.h>
#include <mfidl.h>
#include <guiddef.h>

#include <vector>
#include <map>
#include <array>
#include <set>
#include <queue>
#include <deque>

#include <mutex>
#include <ppltasks.h>
#include <pplawait.h>

#pragma warning(disable : 4244)

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavutil/hwcontext_d3d11va.h>
}

#pragma warning(default : 4244)

#include "StringUtils.h"
#include "MediaSourceConfig.h"
#include "TimeSpanHelpers.h"
#include "AudioStreamInfo.h"
#include "VideoStreamInfo.h"
#include "SubtitleStreamInfo.h"

// Disable debug string output on non-debug build
#if !_DEBUG
#define DebugMessage(x)
#else
#define DebugMessage(x) OutputDebugString(x)
#endif

template<class T>
std::vector<T> inline to_vector(IVector<T> input)
{
    return to_vector(input.GetView())
}

template<class T>
std::vector<T> inline to_vector(IVectorView<T> input)
{
    std::vector<T> output;
    for (auto i : input)
        output.emplace_back(i);
    return output;
}

// Creates a weak handler function proxy to the passed instance function (two arguments, e.g. event handler).
// The class T must implement enable_shared_from_this!
template<class T, typename TSender, typename TArgs>
std::function<void(TSender, TArgs)> inline weak_handler(T* instance, void(T::* instanceMethod)(TSender, TArgs))
{
    auto wr = instance->weak_from_this();
    auto handler = [wr, instanceMethod](TSender sender, TArgs args)
    {
        auto instanceLocked = std::dynamic_pointer_cast<T>(wr.lock());
        if (instanceLocked)
        {
            (instanceLocked.get()->*instanceMethod)(sender, args);
        }
    };
    return handler;
}

// Creates a weak handler function proxy to the passed instance function (no arguments, e.g. dispatcher handler).
// The class T must implement enable_shared_from_this!
template<class T>
std::function<void()> inline weak_handler(T* instance, void(T::* instanceMethod)())
{
    std::weak_ptr<T> wr = instance->weak_from_this();
    auto handler = [wr, instanceMethod]()
    {
        auto instanceLocked = wr.lock();
        if (instanceLocked)
        {
            (instanceLocked.get()->*instanceMethod)();
        }
    };
    return handler;
}
