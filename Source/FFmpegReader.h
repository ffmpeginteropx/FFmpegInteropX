//*****************************************************************************
//
//	Copyright 2015 Microsoft Corporation
//
//	Licensed under the Apache License, Version 2.0 (the "License");
//	you may not use this file except in compliance with the License.
//	You may obtain a copy of the License at
//
//	http ://www.apache.org/licenses/LICENSE-2.0
//
//	Unless required by applicable law or agreed to in writing, software
//	distributed under the License is distributed on an "AS IS" BASIS,
//	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//	See the License for the specific language governing permissions and
//	limitations under the License.
//
//*****************************************************************************

#pragma once

#include "MediaSampleProvider.h"
#include "AvSubtitleContextTrack.h"

namespace FFmpegInteropX
{
    class FFmpegReader
    {
    public:
        virtual ~FFmpegReader();
        int ReadPacket();
        std::function<void(AvSubtitleEventArgs*)> eventCallback;
        std::shared_ptr<std::vector<AvSubtitleContextTrack*>> avSubtitleContextTracks_ptr;
        FFmpegReader(AVFormatContext* avFormatCtx, std::vector<std::shared_ptr<MediaSampleProvider>>* sampleProviders);

    private:
        AVFormatContext* m_pAvFormatCtx = NULL;
        std::vector<std::shared_ptr<MediaSampleProvider>>* sampleProviders = NULL;
    };
}
