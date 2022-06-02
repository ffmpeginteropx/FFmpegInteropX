#pragma once

#include "pch.h"
#include "Enumerations.h"
#include "AudioStreamInfo.g.h"
#include "VideoStreamInfo.g.h"
#include "SubtitleStreamInfo.g.h"
#include "ChapterInfo.g.h"
#include "FormatInfo.g.h"

using namespace winrt;

namespace winrt::FFmpegInteropX::implementation
{
	
	using namespace Windows::Media::Core;
	using namespace Windows::Media::Playback;

	interface IStreamInfo
	{
		hstring Name();
		hstring Language();
		hstring CodecName();
		StreamDisposition Disposition();
		INT64 Bitrate();
		bool IsDefault();
	};

	struct AudioStreamInfo : public AudioStreamInfoT<AudioStreamInfo>, public IStreamInfo
	{
	public:
		AudioStreamInfo(
			hstring name,
			hstring language,
			hstring codecName,
			StreamDisposition disposition,
			INT64 bitrate,
			bool isDefault,
			int channels,
			hstring channelLayout,
			int sampleRate,
			int bitsPerSample,
			DecoderEngine decoderEngine)
		{
			this->name = name;
			this->language = language;
			this->codecName = codecName;
			this->disposition = disposition;
			this->bitrate = bitrate;
			this->isDefault = isDefault;

			this->channels = channels;
			this->channelLayout = channelLayout;
			this->sampleRate = sampleRate;
			this->bitsPerSample = bitsPerSample;

			this->decoderEngine = decoderEngine;
		}

		virtual  hstring Name() { return name; }
		virtual  hstring Language() { return language; }
		virtual  hstring CodecName() { return codecName; }
		virtual  StreamDisposition Disposition() { return disposition; }
		virtual  INT64 Bitrate() { return bitrate; }
		virtual  bool IsDefault() { return isDefault; }
		int Channels() { return channels; }
		hstring ChannelLayout() { return channelLayout; }
		int SampleRate() { return sampleRate; }
		int BitsPerSample() { return bitsPerSample; }

		DecoderEngine DecoderEngine() { return decoderEngine; }

		void SetDefault()
		{
			isDefault = true;
		}

	private:
		hstring name;
		hstring language;
		hstring codecName;
		StreamDisposition disposition;
		INT64 bitrate;
		bool isDefault;

		int channels;
		hstring channelLayout;
		int sampleRate;
		int bitsPerSample;

		winrt::FFmpegInteropX::implementation::DecoderEngine decoderEngine;
	};

	struct VideoStreamInfo : public VideoStreamInfoT<VideoStreamInfo>, public IStreamInfo
	{
	public:
		VideoStreamInfo(hstring name,
			hstring language,
			hstring codecName,
			StreamDisposition disposition,
			INT64 bitrate,
			bool isDefault,
			int pixelWidth,
			int pixelHeight,
			double displayAspectRatio,
			int bitsPerSample,
			double framesPerSecond,
			HardwareDecoderStatus hwAccel,
			DecoderEngine decoderEngine)
		{
			this->name = name;
			this->language = language;
			this->codecName = codecName;
			this->disposition = disposition;
			this->bitrate = bitrate;
			this->isDefault = isDefault;

			this->pixelWidth = pixelWidth;
			this->pixelHeight = pixelHeight;
			this->displayAspectRatio = displayAspectRatio;
			this->bitsPerSample = bitsPerSample;
			this->framesPerSecond = framesPerSecond;

			this->hardwareDecoderStatus = hwAccel;
			this->decoderEngine = decoderEngine;
		}

		virtual  hstring Name() { return name; }
		virtual  hstring Language() { return language; }
		virtual  hstring CodecName() { return codecName; }
		virtual  StreamDisposition Disposition() { return disposition; }
		virtual  INT64 Bitrate() { return bitrate; }
		virtual  bool IsDefault() { return isDefault; }

		int PixelWidth() { return pixelWidth; }
		int PixelHeight() { return pixelHeight; }
		double DisplayAspectRatio() { return displayAspectRatio; }
		int BitsPerSample() { return bitsPerSample; }
		double FramesPerSecond() { return framesPerSecond; }

		///<summary>Override the frame rate of the video stream.</summary>
		///<remarks>
		/// This must be set before calling CreatePlaybackItem().
		/// Setting this can cause A/V desync, since it will only affect this stream.
		/// </remarks>
		double FramesPerSecondOverride()
		{
			return framesPerSecondOverride;
		}

		void FramesPerSecondOverride(double value)
		{
			framesPerSecondOverride = value;
		}

		HardwareDecoderStatus HardwareDecoderStatus() { return hardwareDecoderStatus; }

		DecoderEngine DecoderEngine() { return decoderEngine; }

		void DecoderEngine(winrt::FFmpegInteropX::implementation::DecoderEngine value)
		{
			decoderEngine = value;
		}

		void SetDefault()
		{
			isDefault = true;
		}

	private:
		hstring  name;
		hstring language;
		hstring codecName;
		StreamDisposition disposition;
		INT64 bitrate;
		bool isDefault;

		int pixelWidth;
		int pixelHeight;
		double displayAspectRatio;
		int bitsPerSample;
		double framesPerSecond;
		double framesPerSecondOverride = 0;
		winrt::FFmpegInteropX::implementation::HardwareDecoderStatus hardwareDecoderStatus;
		winrt::FFmpegInteropX::implementation::DecoderEngine decoderEngine;
	};


	struct SubtitleStreamInfo : public SubtitleStreamInfoT<SubtitleStreamInfo>, public IStreamInfo
	{
	public:
		SubtitleStreamInfo(hstring name,
			hstring language,
			hstring codecName,
			StreamDisposition disposition,
			bool isDefault,
			bool isForced,
			TimedMetadataTrack track,
			bool isExternal)
		{
			this->name = name;
			this->language = language;
			this->codecName = codecName;
			this->disposition = disposition;
			this->isDefault = isDefault;
			this->isForced = isForced;
			this->track = track;
			this->isExternal = isExternal;
		}

		virtual  hstring Name() { return name; }
		virtual  hstring Language() { return language; }
		virtual  hstring CodecName() { return codecName; }
		virtual  StreamDisposition Disposition() { return disposition; }
		virtual  INT64 Bitrate() { return 0; }
		virtual  bool IsDefault() { return isDefault; }

		bool IsExternal() { return isExternal; }
		bool IsForced() { return isForced; }

		TimedMetadataTrack SubtitleTrack() {
			return track;
		}

		void SubtitleTrack(TimedMetadataTrack value)
		{
			track = value;
		}

		void SetDefault()
		{
			isDefault = true;
		}

	private:
		hstring  name;
		hstring language;
		hstring codecName;
		StreamDisposition disposition;
		bool isDefault;
		bool isForced;
		TimedMetadataTrack track;
		bool isExternal;
	};

	struct ChapterInfo : ChapterInfoT<ChapterInfo>
	{
	public:
		ChapterInfo(hstring title, Windows::Foundation::TimeSpan startTime, Windows::Foundation::TimeSpan duration)
		{
			this->title = title;
			this->startTime = startTime;
			this->duration = duration;
		}

		hstring Title() { return title; } 
		Windows::Foundation::TimeSpan StartTime() { return startTime; } 
		Windows::Foundation::TimeSpan Duration() { return duration; }

	private:
		hstring title;
		Windows::Foundation::TimeSpan startTime;
		Windows::Foundation::TimeSpan duration;
	};

	struct FormatInfo : FormatInfoT<FormatInfo>
	{
	public:
		FormatInfo(hstring title,
			hstring formatName, 
			Windows::Foundation::TimeSpan duration,
			INT64 bitrate)
		{
			this->title = title;
			this->formatName = formatName;
			this->duration = duration;
			this->bitrate = bitrate;
		}

		hstring Title() { return title; } 
		hstring FormatName() { return formatName; } 
		Windows::Foundation::TimeSpan Duration() { return duration; }
		INT64 Bitrate() { return bitrate; } 

	private:
		hstring title;
		hstring formatName;
		Windows::Foundation::TimeSpan duration;
		INT64 bitrate;
	};
}

namespace winrt::FFmpegInteropX::factory_implementation
{
	struct AudioStreamInfo : AudioStreamInfoT<AudioStreamInfo, implementation::AudioStreamInfo>
	{
	};

	struct VideoStreamInfo : AudioStreamInfoT<VideoStreamInfo, implementation::VideoStreamInfo>
	{
	};

	struct SubtitleStreamInfo : SubtitleStreamInfoT<SubtitleStreamInfo, implementation::SubtitleStreamInfo>
	{
	};

	struct AudioStreamInfo : AudioStreamInfoT<AudioStreamInfo, implementation::AudioStreamInfo>
	{
	};

	struct ChapterInfo : ChapterInfoT<ChapterInfo, implementation::ChapterInfo>
	{
	};

	struct FormatInfo : FormatInfoT<FormatInfo, implementation::FormatInfo>
	{
	};
}