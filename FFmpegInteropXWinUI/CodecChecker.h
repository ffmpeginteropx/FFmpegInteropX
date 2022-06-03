#pragma once
#include "CodecChecker.g.h"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropXWinUI::implementation
{
    struct CodecChecker : CodecCheckerT<CodecChecker>
    {
        CodecChecker() = default;

        static winrt::event_token CodecRequired(FFmpegInteropXWinUI::CodecRequiredEventHandler const& handler);
        static void CodecRequired(winrt::event_token const& token) noexcept;
        static Windows::Foundation::IAsyncAction InitializeAsync();
        static Windows::Foundation::IAsyncAction RefreshAsync();
        static Windows::Foundation::IAsyncOperation<bool> CheckIsMpeg2VideoExtensionInstalledAsync();
        static Windows::Foundation::IAsyncOperation<bool> CheckIsVP9VideoExtensionInstalledAsync();
        static Windows::Foundation::IAsyncOperation<bool> CheckIsHEVCVideoExtensionInstalledAsync();
        static Windows::Foundation::IAsyncOperation<bool> OpenMpeg2VideoExtensionStoreEntryAsync();
        static Windows::Foundation::IAsyncOperation<bool> OpenVP9VideoExtensionStoreEntryAsync();
        static Windows::Foundation::IAsyncOperation<bool> OpenHEVCVideoExtensionStoreEntryAsync();
    };
}
namespace winrt::FFmpegInteropXWinUI::factory_implementation
{
    struct CodecChecker : CodecCheckerT<CodecChecker, implementation::CodecChecker>
    {
    };
}
