#include "pch.h"
#include "SubtitleProvider.h"
#include "FFmpegReader.h"
#include <memory>
#include "winrt/FFmpegInteropX.h"
#include "MediaSourceConfig.h"
#include "winrt/Windows.Media.Core.h"
#include "SubtitleProviderBitmap.h"

using namespace FFmpegInteropX;
using namespace std;
using namespace winrt::FFmpegInteropX;
using namespace winrt::Windows::Media::Core;

//FFmpegInteropX::SubtitleProvider::SubtitleProvider(std::shared_ptr<FFmpegReader> reader,
//	AVFormatContext* avFormatCtx,
//	AVCodecContext* avCodecCtx,
//	MediaSourceConfig const& config,
//	int index,
//	winrt::Windows::Media::Core::TimedMetadataKind const& ptimedMetadataKind,
//	winrt::Windows::UI::Core::CoreDispatcher const& pdispatcher)
//	
//	timedMetadataKind(ptimedMetadataKind),
//	dispatcher(pdispatcher),
//{
//}
