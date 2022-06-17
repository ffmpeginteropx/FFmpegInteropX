#pragma once
#include "MediaThumbnailData.g.h"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropXWinUI::implementation
{
	struct MediaThumbnailData : MediaThumbnailDataT<MediaThumbnailData>
	{
		MediaThumbnailData() = default;

		MediaThumbnailData(Windows::Storage::Streams::IBuffer const& buffer, hstring const& extension);
		Windows::Storage::Streams::IBuffer Buffer();
		hstring Extension();
		void Close();

		winrt::Windows::Storage::Streams::IBuffer _buffer = { nullptr };
		winrt::hstring _extension;
		~MediaThumbnailData();
	};
}
namespace winrt::FFmpegInteropXWinUI::factory_implementation
{
	struct MediaThumbnailData : MediaThumbnailDataT<MediaThumbnailData, implementation::MediaThumbnailData>
	{
	};
}
