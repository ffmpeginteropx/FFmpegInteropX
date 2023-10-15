#pragma once
#include "AudioConfig.g.h"
#include "ConfigurationCommon.h"

namespace winrt::FFmpegInteropX::implementation
{
    struct AudioConfig : AudioConfigT<AudioConfig>
    {
        AudioConfig() = default;

        ///<summary>Enable passthrough for MP3 audio to system decoder.</summary>
               ///<remarks>This could allow hardware decoding on some platforms (e.g. Windows Phone).</remarks>
        PROPERTY(PassthroughAudioMP3, bool, false);

        ///<summary>Enable passthrough for AAC audio to system decoder.</summary>
        ///<remarks>This could allow hardware decoding on some platforms (e.g. Windows Phone).</remarks>
        PROPERTY(PassthroughAudioAAC, bool, false);

        ///<summary>The maximum number of audio decoding threads. Setting to means using the number of logical CPU cores.</summary>
        PROPERTY(MaxAudioThreads, int32_t, 2);

        ///<summary>The default name to use for audio streams.</summary>
        PROPERTY_CONST(DefaultAudioStreamName, hstring, L"Audio Stream");

        ///<summary>Initial FFmpeg audio filters. Might be changed later through FFmpegMediaSource.SetFFmpegAudioFilters().</summary>
        PROPERTY_CONST(FFmpegAudioFilters, hstring, {});

        ///<summary>Downmix multi-channel audio streams to stereo format.</summary>
        PROPERTY(DownmixAudioStreamsToStereo, bool, false);

    };
}
