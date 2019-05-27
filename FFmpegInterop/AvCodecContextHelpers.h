#pragma once
extern "C"
{
#include <libavformat/avformat.h>
}
namespace FFmpegInterop
{

	ref class AvCodecContextHelpers
	{
	internal:
		static int GetNBChannels(AVCodecContext* m_pAvCodecCtx)
		{
			return m_pAvCodecCtx->profile == FF_PROFILE_AAC_HE_V2 && m_pAvCodecCtx->channels == 1 ? 2 : m_pAvCodecCtx->channels;
		}

		static long long GetChannelLayout(AVCodecContext* m_pAvCodecCtx, int inChannels)
		{
			return m_pAvCodecCtx->channel_layout && (m_pAvCodecCtx->profile != FF_PROFILE_AAC_HE_V2 || m_pAvCodecCtx->channels > 1) ? m_pAvCodecCtx->channel_layout : GetDefaultChannelLayout(inChannels);
		}

		static long long GetDefaultChannelLayout(int channels)
		{
			return channels == 6 ? AV_CH_LAYOUT_5POINT1 : av_get_default_channel_layout(channels);
		}

	private:
		AvCodecContextHelpers() {}
	};
}




