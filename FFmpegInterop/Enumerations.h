#pragma once

namespace FFmpegInterop
{
	public enum class HardwareDecoderStatus
	{
		Unknown,
		Available,
		NotAvailable
	};

	public enum class DecoderEngine
	{
		SystemDecoder,
		FFmpegSoftwareDecoder
	};

	public enum class VideoDecoderMode
	{
		///<summary>Automatically detect hardware acceleration capabilities and enable passthrough for those formats.</summary>
		AutoDetection,
		///<summary>Use manual selection for each format (e.g. config.PassthroughVideoH264 = true).</summary>
		ManualSelection,
		///<summary>Force use of system decoder.</summary>
		ForceSystemDecoder,
		///<summary>Force use of ffmpeg software decoder.</summary>
		ForceFFmpegSoftwareDecoder
	};
}