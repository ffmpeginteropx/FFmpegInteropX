#include "pch.h"
#include "CodecChecker.h"
#include "CodecChecker.g.cpp"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropXWinUI::implementation
{
    winrt::event_token CodecChecker::CodecRequired(FFmpegInteropXWinUI::CodecRequiredEventHandler const& handler)
    {
        throw hresult_not_implemented();
    }
    void CodecChecker::CodecRequired(winrt::event_token const& token) noexcept
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::IAsyncAction CodecChecker::InitializeAsync()
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::IAsyncAction CodecChecker::RefreshAsync()
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::IAsyncOperation<bool> CodecChecker::CheckIsMpeg2VideoExtensionInstalledAsync()
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::IAsyncOperation<bool> CodecChecker::CheckIsVP9VideoExtensionInstalledAsync()
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::IAsyncOperation<bool> CodecChecker::CheckIsHEVCVideoExtensionInstalledAsync()
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::IAsyncOperation<bool> CodecChecker::OpenMpeg2VideoExtensionStoreEntryAsync()
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::IAsyncOperation<bool> CodecChecker::OpenVP9VideoExtensionStoreEntryAsync()
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::IAsyncOperation<bool> CodecChecker::OpenHEVCVideoExtensionStoreEntryAsync()
    {
        throw hresult_not_implemented();
    }
}
