#pragma once

namespace FFmpegInteropX
{
	namespace WFM = Windows::Foundation::Metadata;

	public enum class HardwareDecoderStatus
	{
		Unknown,
		Available,
		NotAvailable
	};

	public enum class DecoderEngine
	{
		SystemDecoder,
		FFmpegSoftwareDecoder,
		FFmpegD3D11HardwareDecoder
	};

	public enum class VideoDecoderMode
	{
		///<summary>Use FFmpeg D3D11 hardware acceleration, if available. Otherwise, use FFmpeg software decoder</summary>
		///<remarks>The VideoStreamInfo will show the actual decoder only after playback has started.</remarks>
		Automatic,
		///<summary>Automatically detect hardware acceleration capabilities and use system decoders for those formats.</summary>
		///<remarks>For some formats, installation of video codecs is required for this to work. Check CodecChecker.CodecRequired event.</remarks>
		AutomaticSystemDecoder,
		///<summary>Force use of system decoder.</summary>
		ForceSystemDecoder,
		///<summary>Force use of ffmpeg software decoder.</summary>
		ForceFFmpegSoftwareDecoder
	};
}