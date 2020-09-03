#pragma once

namespace FFmpegInterop
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
		///<summary>Automatically detect hardware acceleration capabilities and use system decoders for those formats.</summary>
		///<remarks>This enum value is deprecated. Use AutomaticSystemDecoder instead.</remarks>
		[WFM::Deprecated("Use the AutomaticSystemDecoder enum value.", WFM::DeprecationType::Deprecate, 0x0)]
		AutoDetection,
		///<summary>Use manual selection for each format (e.g. config.PassthroughVideoH264 = true).</summary>
		///<remarks>Manual selection of system decoders is deprecated.</remarks>
		[WFM::Deprecated("Manual selection of passthrough to system decoders is deprecated. Use VideoDecoderMode.AutomaticSystemDecoder or VideoDecoderMode.ForceSystemDecoder.", WFM::DeprecationType::Deprecate, 0x0)]
		ManualSelection,
		///<summary>Force use of system decoder.</summary>
		ForceSystemDecoder,
		///<summary>Force use of ffmpeg software decoder.</summary>
		ForceFFmpegSoftwareDecoder
	};
}