#pragma once
#include <memory>
#include <unknwn.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Media.Playback.h>
#include <winrt/Windows.Storage.h>
#include <d3d11.h>
#include <queue>
#include "StringUtils.h"
#include <winrt/Microsoft.UI.Composition.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h>
#include <winrt/Microsoft.UI.Xaml.Data.h>
#include <winrt/Microsoft.UI.Xaml.Markup.h>
#include <winrt/Microsoft.UI.Xaml.Navigation.h>
#include <winrt/Microsoft.UI.Dispatching.h>
#include <winrt/microsoft.ui.dispatching.co_await.h>
#include <winrt/Windows.Graphics.DirectX.Direct3D11.h>
#include <winrt/Windows.Graphics.DirectX.h>
#include <winrt/Windows.Globalization.h>
#include <winrt/Windows.Foundation.Metadata.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.Media.Core.h>
#include <winrt/Windows.Media.MediaProperties.h>
#include <windows.graphics.directx.direct3d11.interop.h>
#include <mfidl.h>
#include <robuffer.h>
#include <mfapi.h>
#include <winrt/Windows.UI.Core.h>
#include <guiddef.h>
#include <vector>
#include <map>
#include <array>
#include <set>
#include <winrt/Windows.Storage.Streams.h>
#include <ppltasks.h>
#include <winrt/Windows.UI.h>
#include <winrt/Windows.Storage.FileProperties.h>
#include <winrt/Windows.Media.h>
#include <winrt/Windows.UI.Core.Preview.h>
#include <winrt/Windows.Media.Core.h>

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