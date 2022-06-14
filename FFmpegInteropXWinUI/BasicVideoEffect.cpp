#include "pch.h"
#include "BasicVideoEffect.h"
#include "BasicVideoEffect.g.cpp"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropXWinUI::implementation
{
    void BasicVideoEffect::SetProperties(Windows::Foundation::Collections::IPropertySet const& configuration)
    {
        throw hresult_not_implemented();
    }

    bool BasicVideoEffect::IsReadOnly()
    {
        throw hresult_not_implemented();
    }

    Windows::Media::Effects::MediaMemoryTypes BasicVideoEffect::SupportedMemoryTypes()
    {
        throw hresult_not_implemented();
    }

    bool BasicVideoEffect::TimeIndependent()
    {
        throw hresult_not_implemented();
    }

    Windows::Foundation::Collections::IVectorView<Windows::Media::MediaProperties::VideoEncodingProperties> BasicVideoEffect::SupportedEncodingProperties()
    {
        throw hresult_not_implemented();
    }

    void BasicVideoEffect::SetEncodingProperties(Windows::Media::MediaProperties::VideoEncodingProperties const& encodingProperties, Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice const& device)
    {
        throw hresult_not_implemented();
    }

    void BasicVideoEffect::ProcessFrame(Windows::Media::Effects::ProcessVideoFrameContext const& context)
    {
        throw hresult_not_implemented();
    }

    void BasicVideoEffect::Close(Windows::Media::Effects::MediaEffectClosedReason const& reason)
    {
        throw hresult_not_implemented();
    }

    void BasicVideoEffect::DiscardQueuedFrames()
    {
        throw hresult_not_implemented();
    }
}
