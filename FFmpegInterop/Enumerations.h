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
}