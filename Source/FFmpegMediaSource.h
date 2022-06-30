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
#include <mutex>
#include <collection.h>

#include "Enumerations.h"
#include "FFmpegReader.h"
#include "MediaSampleProvider.h"
#include "MediaThumbnailData.h"
#include "VideoFrame.h"
#include "StreamInfo.h"
#include "SubtitleProvider.h"
#include "AttachedFileHelper.h"
#include "CodecChecker.h"
#include "MediaMetadata.h"

extern "C"
{
#include <libavformat/avformat.h>
}

namespace FFmpegInteropX
{
    using namespace Platform;
    using namespace Windows::Foundation;
    using namespace Windows::Foundation::Collections;
    using namespace Windows::Media::Core;
    using namespace Windows::Media::Playback;
    using namespace Platform::Collections;
    using namespace Windows::UI::Core;
    using namespace Windows::UI::Xaml;
    namespace WFM = Windows::Foundation::Metadata;

    enum ByteOrderMark
    {
        Unchecked,
        Unknown,
        UTF8
    };

    ///<summary>This is the main class that allows media playback with ffmpeg.</summary>
    public ref class FFmpegMediaSource sealed
    {
    public:
        ///<summary>Creates a FFmpegMediaSource from a stream.</summary>
        static IAsyncOperation<FFmpegMediaSource^>^ CreateFromStreamAsync(IRandomAccessStream^ stream, MediaSourceConfig^ config);

        ///<summary>Creates a FFmpegMediaSource from a stream.</summary>
        static IAsyncOperation<FFmpegMediaSource^>^ CreateFromStreamAsync(IRandomAccessStream^ stream) { return CreateFromStreamAsync(stream, ref new MediaSourceConfig()); }

        ///<summary>Creates a FFmpegMediaSource from a Uri.</summary>
        static IAsyncOperation<FFmpegMediaSource^>^ CreateFromUriAsync(String^ uri, MediaSourceConfig^ config);

        ///<summary>Creates a FFmpegMediaSource from a Uri.</summary>
        static IAsyncOperation<FFmpegMediaSource^>^ CreateFromUriAsync(String^ uri) { return CreateFromUriAsync(uri, ref new MediaSourceConfig()); }

        ///<summary>Sets the subtitle delay for all subtitle streams. Use negative values to speed them up, positive values to delay them.</summary>
        void SetSubtitleDelay(TimeSpan delay);

        ///<summary>Sets FFmpeg audio effects. This replaces any effects which were already set.</summary>
        [WFM::DefaultOverload]
        void SetFFmpegAudioFilters(String^ audioFilters);

        ///<summary>Sets FFmpeg video filters. This replaces any filters which were already set.</summary>
        ///<remarks>Using FFmpeg video filters will degrade playback performance, since they run on the CPU and not on the GPU.</remarks>
        [WFM::DefaultOverload]
        void SetFFmpegVideoFilters(String^ videoEffects);

        ///<summary>Disables audio effects.</summary>
        void DisableAudioEffects();

        ///<summary>Disables video effects.</summary>
        void DisableVideoEffects();

        ///<summary>Extracts an embedded thumbnail, if one is available (see HasThumbnail).</summary>
        MediaThumbnailData^ ExtractThumbnail();

        ///<summary>Gets the MediaStreamSource. Using the MediaStreamSource will prevent subtitles from working. Please use CreateMediaPlaybackItem instead.</summary>
        MediaStreamSource^ GetMediaStreamSource();

        ///<summary>Creates a MediaPlaybackItem for playback.</summary>
        MediaPlaybackItem^ CreateMediaPlaybackItem();

        ///<summary>Creates a MediaPlaybackItem for playback which starts at the specified stream offset.</summary>
        MediaPlaybackItem^ CreateMediaPlaybackItem(TimeSpan startTime);

        ///<summary>Creates a MediaPlaybackItem for playback which starts at the specified stream offset and ends after the specified duration.</summary>
        MediaPlaybackItem^ CreateMediaPlaybackItem(TimeSpan startTime, TimeSpan durationLimit);

        ///<summary>Adds an external subtitle from a stream.</summary>
        ///<param name="stream">The subtitle stream.</param>
        ///<param name="streamName">The name to use for the subtitle.</param>
        IAsyncOperation<IVectorView<SubtitleStreamInfo^>^>^ AddExternalSubtitleAsync(IRandomAccessStream^ stream, String^ streamName);

        ///<summary>Adds an external subtitle from a stream.</summary>
        ///<param name="stream">The subtitle stream.</param>
        IAsyncOperation<IVectorView<SubtitleStreamInfo^>^>^ AddExternalSubtitleAsync(IRandomAccessStream^ stream)
        {
            return AddExternalSubtitleAsync(stream, config->DefaultExternalSubtitleStreamName);
        }

        ///<summary>Destroys the FFmpegMediaSource instance and releases all resources.</summary>
        virtual ~FFmpegMediaSource();

        // Properties

        ///<summary>Gets the configuration that has been passed when creating the MSS instance.</summary>
        property MediaSourceConfig^ Configuration
        {
            MediaSourceConfig^ get()
            {
                return config;
            }
        }

        property IMapView<String^, IVectorView<String^>^>^ MetadataTags
        {
            IMapView<String^, IVectorView<String^>^>^ get()
            {
                metadata->LoadMetadataTags(avFormatCtx);
                return metadata->MetadataTags;
            }
        }

        ///<summary>Gets the duration of the stream. Returns zero, if this is streaming media.</summary>
        property TimeSpan Duration
        {
            TimeSpan get()
            {
                return mediaDuration;
            };
        };

        ///<summary>Gets the current video stream information.</summary>
        property VideoStreamInfo^ CurrentVideoStream
        {
            VideoStreamInfo^ get()
            {
                auto stream = currentVideoStream;
                return stream ? stream->VideoInfo : nullptr;
            }
        }

        ///<summary>Gets the current audio stream information.</summary>
        property AudioStreamInfo^ CurrentAudioStream
        {
            AudioStreamInfo^ get()
            {
                auto stream = currentAudioStream;
                return stream ? stream->AudioInfo : nullptr;
            }
        }

        ///<summary>Gets video stream information</summary>
        property IVectorView<VideoStreamInfo^>^ VideoStreams
        {
            IVectorView<VideoStreamInfo^>^ get() { return videoStreamInfos; }
        }

        ///<summary>Gets audio stream information.</summary>
        property IVectorView<AudioStreamInfo^>^ AudioStreams
        {
            IVectorView<AudioStreamInfo^>^ get() { return audioStreamInfos; }
        }

        ///<summary>Gets subtitle stream information.</summary>
        property IVectorView<SubtitleStreamInfo^>^ SubtitleStreams
        {
            IVectorView<SubtitleStreamInfo^>^ get() { return subtitleStreamInfos; }
        }

        ///<summary>Gets chapter information.</summary>
        property IVectorView<ChapterInfo^>^ ChapterInfos
        {
            IVectorView<ChapterInfo^>^ get() { return chapterInfos; }
        }

        ///<summary>Gets chapter information.</summary>
        property FormatInfo^ FormatInfo
        {
            FFmpegInteropX::FormatInfo^ get() { return formatInfo; }
        }

        ///<summary>Gets a boolean indication if a thumbnail is embedded in the file.</summary>
        property bool HasThumbnail
        {
            bool get() { return thumbnailStreamIndex; }
        }

        ///<summary>Gets the MediaPlaybackItem that was created before by using CreateMediaPlaybackItem.</summary>
        property MediaPlaybackItem^ PlaybackItem
        {
            MediaPlaybackItem^ get()
            {
                return playbackItem;
            }
        }

        ///<summary>The current subtitle delay used by this instance.</summary>
        property TimeSpan SubtitleDelay
        {
            TimeSpan get() { return subtitleDelay; }
        }

        ///<summary>Gets or sets the BufferTime of the MediaStreamSource.</summary>
        ///<remarks>A value of 0 is recommended for local files, streaming sources should use higher values.</remarks>
        property TimeSpan BufferTime
        {
            TimeSpan get() { return mss->BufferTime; }
            void set(TimeSpan value) { mss->BufferTime = value; }
        }

        ///<summary>Gets or sets the MediaPlaybackSession associated with this FFmpeg source. Used when FastSeek is enabled.</summary>
        ///<remarks>After playback has started, please assign MediaPlayer.PlaybackSession to this property.</remarks>
        property Windows::Media::Playback::MediaPlaybackSession^ PlaybackSession
        {
            MediaPlaybackSession^ get() { return session; }
            void set(MediaPlaybackSession^ value)
            {
                mutexGuard.lock();
                if (session)
                {
                    session->PositionChanged -= sessionPositionEvent;
                }
                session = value;
                if (value)
                {
                    sessionPositionEvent = value->PositionChanged += ref new Windows::Foundation::TypedEventHandler<Windows::Media::Playback::MediaPlaybackSession^, Platform::Object^>(this, &FFmpegInteropX::FFmpegMediaSource::OnPositionChanged);
                }
                mutexGuard.unlock();
            }
        }

    private:
        FFmpegMediaSource(MediaSourceConfig^ config, CoreDispatcher^ dispatcher);

        HRESULT CreateMediaStreamSource(IRandomAccessStream^ stream);
        HRESULT CreateMediaStreamSource(String^ uri);
        HRESULT InitFFmpegContext();
        MediaSource^ CreateMediaSource();
        MediaSampleProvider^ CreateAudioStream(AVStream* avStream, int index);
        MediaSampleProvider^ CreateVideoStream(AVStream* avStream, int index);
        SubtitleProvider^ CreateSubtitleSampleProvider(AVStream* avStream, int index);
        MediaSampleProvider^ CreateAudioSampleProvider(AVStream* avStream, AVCodecContext* avCodecCtx, int index);
        MediaSampleProvider^ CreateVideoSampleProvider(AVStream* avStream, AVCodecContext* avCodecCtx, int index);
        HRESULT ParseOptions(PropertySet^ ffmpegOptions);
        void OnStarting(MediaStreamSource^ sender, MediaStreamSourceStartingEventArgs^ args);
        void OnSampleRequested(MediaStreamSource^ sender, MediaStreamSourceSampleRequestedEventArgs^ args);
        void CheckVideoDeviceChanged();
        void OnSwitchStreamsRequested(MediaStreamSource^ sender, MediaStreamSourceSwitchStreamsRequestedEventArgs^ args);
        void OnAudioTracksChanged(MediaPlaybackItem^ sender, IVectorChangedEventArgs^ args);
        void OnPresentationModeChanged(MediaPlaybackTimedMetadataTrackList^ sender, TimedMetadataPresentationModeChangedEventArgs^ args);
        void InitializePlaybackItem(MediaPlaybackItem^ playbackitem);
        bool CheckUseHardwareAcceleration(AVCodecContext* avCodecCtx, HardwareAccelerationStatus^ status, HardwareDecoderStatus& hardwareDecoderStatus, int maxProfile, int maxLevel);

    internal:
        static FFmpegMediaSource^ CreateFromStream(IRandomAccessStream^ stream, MediaSourceConfig^ config, CoreDispatcher^ dispatcher);
        static FFmpegMediaSource^ CreateFromUri(String^ uri, MediaSourceConfig^ config, CoreDispatcher^ dispatcher);
        static FFmpegMediaSource^ CreateFromUri(String^ uri, MediaSourceConfig^ config);
        HRESULT Seek(TimeSpan position, TimeSpan& actualPosition, bool allowFastSeek);

        property MediaSampleProvider^ VideoSampleProvider
        {
            MediaSampleProvider^ get()
            {
                return currentVideoStream;
            }
        }

        FFmpegReader^ m_pReader;
        AVDictionary* avDict;
        AVIOContext* avIOCtx;
        AVFormatContext* avFormatCtx;
        IStream* fileStreamData;
        ByteOrderMark streamByteOrderMark;
        MediaSourceConfig^ config;

    private:

        MediaStreamSource^ mss;
        EventRegistrationToken startingRequestedToken;
        EventRegistrationToken sampleRequestedToken;
        EventRegistrationToken switchStreamRequestedToken;
        MediaPlaybackItem^ playbackItem;
        Vector<AudioStreamInfo^>^ audioStrInfos;
        Vector<SubtitleStreamInfo^>^ subtitleStrInfos;
        Vector<VideoStreamInfo^>^ videoStrInfos;

        std::vector<MediaSampleProvider^> sampleProviders;
        std::vector<MediaSampleProvider^> audioStreams;
        std::vector<SubtitleProvider^> subtitleStreams;
        std::vector<MediaSampleProvider^> videoStreams;

        MediaSampleProvider^ currentVideoStream;
        MediaSampleProvider^ currentAudioStream;
        String^ currentAudioEffects;
        int thumbnailStreamIndex;

        EventRegistrationToken audioTracksChangedToken;
        EventRegistrationToken subtitlePresentationModeChangedToken;


        IVectorView<VideoStreamInfo^>^ videoStreamInfos;
        IVectorView<AudioStreamInfo^>^ audioStreamInfos;
        IVectorView<SubtitleStreamInfo^>^ subtitleStreamInfos;
        IVectorView<ChapterInfo^>^ chapterInfos;
        FFmpegInteropX::FormatInfo^ formatInfo;

        AttachedFileHelper^ attachedFileHelper;

        MediaMetadata^ metadata;

        std::recursive_mutex mutexGuard;
        CoreDispatcher^ dispatcher;
        MediaPlaybackSession^ session;
        EventRegistrationToken sessionPositionEvent;
        TimeSpan currentPosition;
        TimeSpan lastPosition;
        bool isFirstSeekAfterStreamSwitch;

        String^ videoCodecName;
        String^ audioCodecName;
        TimeSpan mediaDuration;
        TimeSpan subtitleDelay;
        unsigned char* fileStreamBuffer;
        bool isFirstSeek;
        AVBufferRef* avHardwareContext;
        AVBufferRef* avHardwareContextDefault;
        ID3D11Device* device;
        ID3D11DeviceContext* deviceContext;
        HANDLE deviceHandle;
        IMFDXGIDeviceManager* deviceManager;

        static CoreDispatcher^ GetCurrentDispatcher();
        void OnPositionChanged(Windows::Media::Playback::MediaPlaybackSession^ sender, Platform::Object^ args);
};

}
