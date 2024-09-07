#pragma once
#include "AudioConfig.g.h"
#include "ConfigurationCommon.h"

namespace winrt::FFmpegInteropX::implementation
{
    struct AudioConfig : AudioConfigT<AudioConfig>
    {
        AudioConfig() = default;

        ///<summary>Use system decoder for MP3 decoding.</summary>
        ///<remarks>This could allow hardware decoding on some platforms (e.g. Hololens). Not recommended for PC.</remarks>
        PROPERTY(SystemDecoderMP3, bool, false);

        ///<summary>Use system decoder for AAC audio.</summary>
        ///<remarks>This could allow hardware decoding on some platforms (e.g. Hololens). Not recommended for PC.</remarks>
        PROPERTY(SystemDecoderAAC, bool, false);

        ///<summary>The maximum number of audio decoding threads. Setting to 0 means using the number of logical CPU cores.</summary>
        PROPERTY(MaxDecoderThreads, int32_t, 2);

        ///<summary>The default name to use for audio streams.</summary>
        PROPERTY_CONST(DefaultStreamName, hstring, L"Audio Stream");

        ///<summary>Initial FFmpeg audio filters. Might be changed later through FFmpegMediaSource.SetFFmpegAudioFilters().</summary>
        PROPERTY_CONST(FFmpegAudioFilters, hstring, {});

        ///<summary>Downmix multi-channel audio streams to stereo format.</summary>
        PROPERTY(DownmixAudioStreamsToStereo, bool, false);

    };
}
