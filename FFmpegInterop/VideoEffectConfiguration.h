#pragma once
namespace FFmpegInterop
{
	public ref class VideoEffectConfiguration sealed
	{
	public:
		property float Brightness;

		property float Contrast;

		property float Saturation;

		property float Temperature;

		property float Tint;

		property float Sharpness;

		VideoEffectConfiguration()
		{
			Sharpness = 0.0f;
			Contrast = 1.0f;
			Brightness = 1.0f;
			Saturation = 1.0f;
			Tint = 0.0f;
			Temperature = 0.0f;
		}
	};
}