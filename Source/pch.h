#pragma once
#include <unknwn.h>
#include <shcore.h>
#include <memory>

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
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
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=nullptr; } }

template<class T>
std::vector<T> to_vector(IVector<T> input)
{
    return to_vector(input.GetView())
}

template<class T>
std::vector<T> to_vector(IVectorView<T> input)
{
    std::vector<T> output;
    for (auto i : input)
        output.emplace_back(i);
    return output;
}
