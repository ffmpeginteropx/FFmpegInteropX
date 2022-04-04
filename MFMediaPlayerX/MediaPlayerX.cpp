#include "pch.h"
#include "MediaPlayerX.h"
#if __has_include("MediaPlayerX.g.cpp")
#include "MediaPlayerX.g.cpp"
#endif

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::MFMediaPlayerX::implementation
{
    int32_t MediaPlayerX::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void MediaPlayerX::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }
}
