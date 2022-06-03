#pragma once
#include "BasicVideoEffect.g.h"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropXWinUI::implementation
{
    struct BasicVideoEffect : BasicVideoEffectT<BasicVideoEffect>
    {
        BasicVideoEffect() = default;

        void SetProperties(Windows::Foundation::Collections::IPropertySet const& configuration);
        bool IsReadOnly();
        Windows::Media::Effects::MediaMemoryTypes SupportedMemoryTypes();
        bool TimeIndependent();
        Windows::Foundation::Collections::IVectorView<Windows::Media::MediaProperties::VideoEncodingProperties> SupportedEncodingProperties();
        void SetEncodingProperties(Windows::Media::MediaProperties::VideoEncodingProperties const& encodingProperties, Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice const& device);
        void ProcessFrame(Windows::Media::Effects::ProcessVideoFrameContext const& context);
        void Close(Windows::Media::Effects::MediaEffectClosedReason const& reason);
        void DiscardQueuedFrames();
    };
}
