#pragma once

#include "pch.h"
#include "Enumerations.h"

using namespace Platform;

namespace FFmpegInterop
{
	public interface class IStreamInfo
	{
		property String^ Name { String^ get(); }
		property String^ Language { String^ get(); }
		property String^ CodecName { String^ get(); }
		property int64 Bitrate { int64 get(); }
		property bool IsDefault { bool get(); }
	};

	public ref class AudioStreamInfo sealed : public IStreamInfo
	{
	public:
		AudioStreamInfo(String^ name, String^ language, String^ codecName, int64 bitrate, bool isDefault,
			int channels, int sampleRate, int bitsPerSample, DecoderEngine decoderEngine)
		{
			this->name = name;
			this->language = language;
			this->codecName = codecName;
			this->bitrate = bitrate;
			this->isDefault = isDefault;

			this->channels = channels;
			this->sampleRate = sampleRate;
			this->bitsPerSample = bitsPerSample;
	
			this->decoderEngine = decoderEngine;
		}

		virtual property String^ Name { String^ get() { return name; } }
		virtual property String^ Language { String^ get() { return language; } }
		virtual property String^ CodecName { String^ get() { return codecName; } }
		virtual property int64 Bitrate { int64 get() { return bitrate; } }
		virtual property bool IsDefault { bool get() { return isDefault; } }

		property int Channels { int get() { return channels; } }
		property int SampleRate { int get() { return sampleRate; } }
		property int BitsPerSample { int get() { return bitsPerSample; } }

		property FFmpegInterop::DecoderEngine DecoderEngine {FFmpegInterop::DecoderEngine get() { return decoderEngine; }}

	internal:
		void SetDefault()
		{
			isDefault = true;
		}

	private:
		String ^ name;
		String^ language;
		String^ codecName;
		int64 bitrate;
		bool isDefault;

		int channels;
		int sampleRate;
		int bitsPerSample;
	
		FFmpegInterop::DecoderEngine decoderEngine;
	};

	public ref class VideoStreamInfo sealed : public IStreamInfo
	{
	public:
		VideoStreamInfo(String^ name, String^ language, String^ codecName, int64 bitrate, bool isDefault,
			int pixelWidth, int pixelHeight, double displayAspectRatio, int bitsPerSample, double framesPerSecond, HardwareDecoderStatus hwAccel, DecoderEngine decoderEngine)
		{
			this->name = name;
			this->language = language;
			this->codecName = codecName;
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

		virtual property String^ Name { String^ get() { return name; } }
		virtual property String^ Language { String^ get() { return language; } }
		virtual property String^ CodecName { String^ get() { return codecName; } }
		virtual property int64 Bitrate { int64 get() { return bitrate; } }
		virtual property bool IsDefault { bool get() { return isDefault; } }

		property int PixelWidth { int get() { return pixelWidth; } }
		property int PixelHeight { int get() { return pixelHeight; } }
		property double DisplayAspectRatio { double get() { return displayAspectRatio; } }
		property int BitsPerSample { int get() { return bitsPerSample; } }
		property double FramesPerSecond { double get() { return framesPerSecond; } }

		///<summary>Override the frame rate of the video stream.</summary>
		///<remarks>
		/// This must be set before calling CreatePlaybackItem().
		/// Setting this can cause A/V desync, since it will only affect this stream.
		/// </remarks>
		property double FramesPerSecondOverride;

		property FFmpegInterop::HardwareDecoderStatus HardwareDecoderStatus {FFmpegInterop::HardwareDecoderStatus get() { return hardwareDecoderStatus; }}
		property FFmpegInterop::DecoderEngine DecoderEngine 
		{
			FFmpegInterop::DecoderEngine get() { return decoderEngine; }
		internal:
			void set(FFmpegInterop::DecoderEngine value) { decoderEngine = value; }
		}

	internal:
		void SetDefault()
		{
			isDefault = true;
		}
	private:
		String ^ name;
		String^ language;
		String^ codecName;
		int64 bitrate;
		bool isDefault;

		int pixelWidth;
		int pixelHeight;
		double displayAspectRatio;
		int bitsPerSample;
		double framesPerSecond;

		FFmpegInterop::HardwareDecoderStatus hardwareDecoderStatus;
		FFmpegInterop::DecoderEngine decoderEngine;
	};


	public ref class SubtitleStreamInfo sealed : public IStreamInfo
	{
	public:
		SubtitleStreamInfo(String^ name, String^ language, String^ codecName, bool isDefault, bool isForced, TimedMetadataTrack^ track, bool isExternal)
		{
			this->name = name;
			this->language = language;
			this->codecName = codecName;
			this->isDefault = isDefault;
			this->isForced = isForced;
			this->track = track;
			this->isExternal = isExternal;
		}

		virtual property String^ Name { String^ get() { return name; } }
		virtual property String^ Language { String^ get() { return language; } }
		virtual property String^ CodecName { String^ get() { return codecName; } }
		virtual property int64 Bitrate { int64 get() { return 0; } }
		virtual property bool IsDefault { bool get() { return isDefault; } }

		property bool IsExternal {bool get() { return isExternal; }}
		property bool IsForced { bool get() { return isForced; } }

		property TimedMetadataTrack^ SubtitleTrack
		{
			TimedMetadataTrack^ get()
			{
				return track;
			}

		internal:
			void set(TimedMetadataTrack^ value)
			{
				track = value;
			}
		}

	internal:
		void SetDefault()
		{
			isDefault = true;
		}

	private:
		String ^ name;
		String^ language;
		String^ codecName;
		bool isDefault;
		bool isForced;
		TimedMetadataTrack^ track;
		bool isExternal;
	};

	public ref class ChapterInfo sealed
	{
	public:
		ChapterInfo(String^ title, TimeSpan startTime, TimeSpan duration)
		{
			this->title = title;
			this->startTime = startTime;
			this->duration = duration;
		}

		property String^ Title { String^ get() { return title; } }
		property TimeSpan StartTime { TimeSpan get() { return startTime; } }
		property TimeSpan Duration { TimeSpan get() { return duration; } }

	private:
		String^ title;
		TimeSpan startTime;
		TimeSpan duration;
	};
}