#pragma once
#include "GeneralConfig.g.h"
#include "ConfigurationCommon.h"

namespace winrt::FFmpegInteropX::implementation
{
    using namespace winrt::Windows::Foundation;

    struct GeneralConfig : GeneralConfigT<GeneralConfig>
    {
        GeneralConfig() = default;

        ///<summary>Automatically extend the duration of the MediaStreamSource, if the file unexpectedly contains additional data.</summary>
        PROPERTY(AutoExtendDuration, bool, true);

        ///<summary>The maximum number of broken frames or packets to skip in a stream before stopping decoding.</summary>
        PROPERTY(SkipErrors, int32_t, 50);

        ///<summary>The maximum supported playback rate. This is set on the media stream source itself. 
        /// This does not modify what the transport control default UI shows as available playback speeds. Custom UI is necessary!</summary>
        PROPERTY(MaxSupportedPlaybackRate, double, 4.0);

        ///<summary>The maximum number of bytes to read in one chunk for Windows.Storage.Streams.IRandomAccessStream sources.</summary>
        PROPERTY(FileStreamReadSize, int32_t, 16384);

        ///<summary>Enables or disables the read-ahead buffer.</summary>
        ///<remarks>This value can be changed any time during playback.</remarks>
        PROPERTY(ReadAheadBufferEnabled, bool, false);

        ///<summary>The maximum number of bytes to buffer ahead per stream.</summary>
        ///<remarks>This value can be changed any time during playback.</remarks>
        PROPERTY(ReadAheadBufferSize, int64_t, 100 * 1024 * 1024);

        ///<summary>The maximum duration to buffer ahead per stream.</summary>
        ///<remarks>This value can be changed any time during playback.</remarks>
        PROPERTY_CONST(ReadAheadBufferDuration, TimeSpan, TimeSpan{ 600000000 });

        /// <summary>FFmpegMediaSource will seek to the closest video keyframe, if set to true.</summary>
        /// <remarks>
        /// For FastSeek to work, you must use the MediaPlayer for playback, and assign
        /// MediaPlayer.PlaybackSession to the FFmpegMediaSource.PlaybackSession property.
        /// </remarks>
        PROPERTY(FastSeek, bool, true);

        ///<summary>Ensure that audio plays without artifacts after fast seeking.</summary>
        ///<remarks>This will slightly reduce the speed of fast seeking. Enabled by default.</remarks>
        PROPERTY(FastSeekCleanAudio, bool, true);

        ///<summary>Try to improve stream switching times when FastSeek is enabled.</summary>
        PROPERTY(FastSeekSmartStreamSwitching, bool, true);

        ///<summary>The folder where attachments such as fonts are stored (inside the app's temp folder).</summary>
        PROPERTY_CONST(AttachmentCacheFolderName, hstring, L"FFmpegAttachmentCache");

        ///<summary>Keep metadata available after MediaSource was closed.</summary>
        ///<remarks>Set this to false to cleanup more memory automatically, if you are sure you don't need metadata after playback end.</remarks>
        PROPERTY(KeepMetadataOnMediaSourceClosed, bool, true);

        ///<summary>Allows applications to apply custom logic to exclude streams from the MediaStreamSource or MediaPlaybackItem.</summary>
        PROPERTY_CONST(MediaStreamFilter, winrt::FFmpegInteropX::IMediaStreamFilter, nullptr);
    };
}
