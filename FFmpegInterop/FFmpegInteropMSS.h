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
#include <queue>
#include <mutex>
#include <pplawait.h>
#include "FFmpegReader.h"
#include "MediaSampleProvider.h"
#include "MediaThumbnailData.h"
#include "VideoFrame.h"
#include "AvEffectDefinition.h"
#include "StreamInfo.h"
#include "SubtitleProvider.h"
#include "AttachedFileHelper.h"
#include "CodecChecker.h"
#include <collection.h>

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Media::Core;
using namespace Windows::Media::Playback;
using namespace Platform::Collections;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml;
namespace WFM = Windows::Foundation::Metadata;

extern "C"
{
#include <libavformat/avformat.h>
}

namespace FFmpegInterop
{
	enum ByteOrderMark
	{
		Unchecked,
		Unknown,
		UTF8
	};

	///<summary>This is the main class that allows media playback with ffmpeg.</summary>
	public ref class FFmpegInteropMSS sealed
	{
	public:
		///<summary>Creates a FFmpegInteropMSS from a stream.</summary>
		static IAsyncOperation<FFmpegInteropMSS^>^ CreateFromStreamAsync(IRandomAccessStream^ stream, FFmpegInteropConfig^ config);

		///<summary>Creates a FFmpegInteropMSS from a stream.</summary>
		static IAsyncOperation<FFmpegInteropMSS^>^ CreateFromStreamAsync(IRandomAccessStream^ stream) { return CreateFromStreamAsync(stream, ref new FFmpegInteropConfig()); }

		///<summary>Creates a FFmpegInteropMSS from a Uri.</summary>
		static IAsyncOperation<FFmpegInteropMSS^>^ CreateFromUriAsync(String^ uri, FFmpegInteropConfig^ config);
		
		///<summary>Creates a FFmpegInteropMSS from a Uri.</summary>
		static IAsyncOperation<FFmpegInteropMSS^>^ CreateFromUriAsync(String^ uri) { return CreateFromUriAsync(uri, ref new FFmpegInteropConfig()); }

		[WFM::Deprecated("Use the CreateFromStreamAsync method.", WFM::DeprecationType::Deprecate, 0x0)]
		static FFmpegInteropMSS^ CreateFFmpegInteropMSSFromStream(IRandomAccessStream^ stream, bool forceAudioDecode, bool forceVideoDecode, PropertySet^ ffmpegOptions, MediaStreamSource^ mss);
		[WFM::Deprecated("Use the CreateFromStreamAsync method.", WFM::DeprecationType::Deprecate, 0x0)]
		static FFmpegInteropMSS^ CreateFFmpegInteropMSSFromStream(IRandomAccessStream^ stream, bool forceAudioDecode, bool forceVideoDecode, PropertySet^ ffmpegOptions);
		[WFM::Deprecated("Use the CreateFromStreamAsync method.", WFM::DeprecationType::Deprecate, 0x0)]
		static FFmpegInteropMSS^ CreateFFmpegInteropMSSFromStream(IRandomAccessStream^ stream, bool forceAudioDecode, bool forceVideoDecode);

		[WFM::Deprecated("Use the CreateFromUriAsync method.", WFM::DeprecationType::Deprecate, 0x0)]
		static FFmpegInteropMSS^ CreateFFmpegInteropMSSFromUri(String^ uri, bool forceAudioDecode, bool forceVideoDecode, PropertySet^ ffmpegOptions);
		[WFM::Deprecated("Use the CreateFromUriAsync method.", WFM::DeprecationType::Deprecate, 0x0)]
		static FFmpegInteropMSS^ CreateFFmpegInteropMSSFromUri(String^ uri, bool forceAudioDecode, bool forceVideoDecode);

		///<summary>Sets the subtitle delay for all subtitle streams. Use negative values to speed them up, positive values to delay them.</summary>
		void SetSubtitleDelay(TimeSpan delay);

		///<summary>Sets audio effects. This replaces any effects which were already set.</summary>
		void SetAudioEffects(IVectorView<AvEffectDefinition^>^ audioEffects);
		
		///<summary>Sets video effects. This replaces any effects which were already set.</summary>
		void SetVideoEffects(IVectorView<AvEffectDefinition^>^ videoEffects);
		
		
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

		///<summary>Destroys the FFmpegInteropMSS instance and releases all resources.</summary>
		virtual ~FFmpegInteropMSS();

		// Properties

		///<summary>Gets the configuration that has been passed when creating the MSS instance.</summary>
		property FFmpegInteropConfig^ Configuration
		{
			FFmpegInteropConfig^ get()
			{
				return config;
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

		///<summary>Gets video stream information.</summary>
		property VideoStreamInfo^ VideoStream
		{
			VideoStreamInfo^ get() { return videoStreamInfo; }
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

		///<summary>Gets a boolean indication if a thumbnail is embedded in the file.</summary>
		property bool HasThumbnail
		{
			bool get() { return thumbnailStreamIndex; }
		}

		[WFM::Deprecated("Use the AudioStreams property.", WFM::DeprecationType::Deprecate, 0x0)]
		property AudioStreamDescriptor^ AudioDescriptor
		{
			AudioStreamDescriptor^ get()
			{
				return currentAudioStream ? dynamic_cast<AudioStreamDescriptor^>(currentAudioStream->StreamDescriptor) : nullptr;
			};
		};

		[WFM::Deprecated("Use the VideoStream property.", WFM::DeprecationType::Deprecate, 0x0)]
		property VideoStreamDescriptor^ VideoDescriptor
		{
			VideoStreamDescriptor^ get()
			{
				return videoStream ? dynamic_cast<VideoStreamDescriptor^>(videoStream->StreamDescriptor) : nullptr;
			};
		};

		[WFM::Deprecated("Use the VideoStream property.", WFM::DeprecationType::Deprecate, 0x0)]
		property String^ VideoCodecName
		{
			String^ get()
			{
				return videoStream ? videoStream->CodecName : nullptr;
			};
		};

		[WFM::Deprecated("Use the AudioStreams property.", WFM::DeprecationType::Deprecate, 0x0)]
		property String^ AudioCodecName
		{
			String^ get()
			{
				return audioStreamInfos->Size > 0 ? audioStreamInfos->GetAt(0)->CodecName : nullptr;
			};
		};

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


	private:
		FFmpegInteropMSS(FFmpegInteropConfig^ config, CoreDispatcher^ dispatcher);

		HRESULT CreateMediaStreamSource(IRandomAccessStream^ stream, MediaStreamSource^ mss);
		HRESULT CreateMediaStreamSource(String^ uri);
		HRESULT InitFFmpegContext();
		MediaSource^ CreateMediaSource();
		MediaSampleProvider^ CreateAudioStream(AVStream * avStream, int index);
		MediaSampleProvider^ CreateVideoStream(AVStream * avStream, int index);
		SubtitleProvider^ CreateSubtitleSampleProvider(AVStream * avStream, int index);
		MediaSampleProvider^ CreateAudioSampleProvider(AVStream * avStream, AVCodecContext* avCodecCtx, int index);
		MediaSampleProvider^ CreateVideoSampleProvider(AVStream * avStream, AVCodecContext* avCodecCtx, int index);
		HRESULT ParseOptions(PropertySet^ ffmpegOptions);
		void OnStarting(MediaStreamSource ^sender, MediaStreamSourceStartingEventArgs ^args);
		void OnSampleRequested(MediaStreamSource ^sender, MediaStreamSourceSampleRequestedEventArgs ^args);
		void OnSwitchStreamsRequested(MediaStreamSource^ sender, MediaStreamSourceSwitchStreamsRequestedEventArgs^ args);
		void OnAudioTracksChanged(MediaPlaybackItem ^sender, IVectorChangedEventArgs ^args);
		void OnPresentationModeChanged(MediaPlaybackTimedMetadataTrackList ^sender, TimedMetadataPresentationModeChangedEventArgs ^args);
		void InitializePlaybackItem(MediaPlaybackItem^ playbackitem);
		bool CheckUseHardwareAcceleration(AVCodecContext* avCodecCtx, HardwareAccelerationStatus^ status, bool manualStatus, int maxProfile, int maxLevel);
		void CheckVideoResolution(bool& result, int width, int height, VideoResolution maxResolution);

	internal:
		static FFmpegInteropMSS^ CreateFromStream(IRandomAccessStream^ stream, FFmpegInteropConfig^ config, MediaStreamSource^ mss, CoreDispatcher^ dispatcher);
		static FFmpegInteropMSS^ CreateFromUri(String^ uri, FFmpegInteropConfig^ config, CoreDispatcher^ dispatcher);
		HRESULT Seek(TimeSpan position);

		property MediaSampleProvider^ VideoSampleProvider
		{
			MediaSampleProvider^ get()
			{
				return videoStream;
			}
		}

		FFmpegReader^ m_pReader;
		AVDictionary * avDict;
		AVIOContext* avIOCtx;
		AVFormatContext* avFormatCtx;
		IStream* fileStreamData;
		ByteOrderMark streamByteOrderMark;
		FFmpegInteropConfig ^ config;

	private:

		MediaStreamSource ^ mss;
		EventRegistrationToken startingRequestedToken;
		EventRegistrationToken sampleRequestedToken;
		EventRegistrationToken switchStreamRequestedToken;
		MediaPlaybackItem^ playbackItem;
		Vector<AudioStreamInfo^>^ audioStrInfos;
		Vector<SubtitleStreamInfo^>^ subtitleStrInfos;

		std::vector<MediaSampleProvider^> sampleProviders;
		std::vector<MediaSampleProvider^> audioStreams;
		std::vector<SubtitleProvider^> subtitleStreams;
		MediaSampleProvider^ videoStream;
		MediaSampleProvider^ currentAudioStream;
		IVectorView<AvEffectDefinition^>^ currentAudioEffects;
		int thumbnailStreamIndex;

		EventRegistrationToken audioTracksChangedToken;
		EventRegistrationToken subtitlePresentationModeChangedToken;

		VideoStreamInfo^ videoStreamInfo;
		IVectorView<AudioStreamInfo^>^ audioStreamInfos;
		IVectorView<SubtitleStreamInfo^>^ subtitleStreamInfos;

		AttachedFileHelper^ attachedFileHelper;

		std::recursive_mutex mutexGuard;
		CoreDispatcher^ dispatcher;
		

		String^ videoCodecName;
		String^ audioCodecName;
		TimeSpan mediaDuration;
		TimeSpan subtitleDelay;
		unsigned char* fileStreamBuffer;
		bool isFirstSeek;

		static CoreDispatcher^ GetCurrentDispatcher();
	};

}
