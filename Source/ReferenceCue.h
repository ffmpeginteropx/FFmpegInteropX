#pragma once
#include<pch.h>

namespace FFmpegInteropX
{
    using namespace Platform;
    using namespace Windows::Foundation;
    using namespace Windows::Media::Core;

    ref class ReferenceCue : IMediaCue
    {
    internal:

        ReferenceCue(IMediaCue^ other)
        {
            this->CueRef = other;
            this->Duration = other->Duration;
            this->Id = other->Id;
            this->StartTime = other->StartTime;
        }

    public:
        virtual property String^ Id;

        virtual property TimeSpan Duration;

        virtual property TimeSpan StartTime;

        property IMediaCue^ CueRef;
    };
}




