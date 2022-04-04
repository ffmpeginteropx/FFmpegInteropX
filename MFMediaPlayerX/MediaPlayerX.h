#pragma once

#include "MediaPlayerX.g.h"

namespace winrt::MFMediaPlayerX::implementation
{
    struct MediaPlayerX : MediaPlayerXT<MediaPlayerX>
    {
        MediaPlayerX() = default;

        int32_t MyProperty();
        void MyProperty(int32_t value);
    };
}

namespace winrt::MFMediaPlayerX::factory_implementation
{
    struct MediaPlayerX :MediaPlayerXT<MediaPlayerX, implementation::MediaPlayerX>
    {
    };
}
