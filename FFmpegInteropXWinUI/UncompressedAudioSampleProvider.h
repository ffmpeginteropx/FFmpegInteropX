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
#include <winrt/FFmpegInteropXWinUI.h>
#include "UncompressedSampleProvider.h"

namespace FFmpegInteropX
{
	class UncompressedAudioSampleProvider : public UncompressedSampleProvider
	{
	public:
		virtual ~UncompressedAudioSampleProvider();

		UncompressedAudioSampleProvider(
			std::shared_ptr<FFmpegReader> reader,
			AVFormatContext* avFormatCtx,
			AVCodecContext* avCodecCtx,
			winrt::FFmpegInteropXWinUI::MediaSourceConfig const& config,
			int streamIndex);
		virtual HRESULT CreateBufferFromFrame(IBuffer* pBuffer, IDirect3DSurface* surface, AVFrame* avFrame, int64_t& framePts, int64_t& frameDuration) override;
		winrt::Windows::Media::Core::IMediaStreamDescriptor CreateStreamDescriptor() override;
		HRESULT CheckFormatChanged(AVSampleFormat format, int channels, UINT64 channelLayout, int sampleRate);
		HRESULT UpdateResampler();


	private:
		SwrContext* m_pSwrCtx = NULL;
		AVSampleFormat inSampleFormat, outSampleFormat;
		int inSampleRate = 0, outSampleRate = 0, inChannels = 0, outChannels = 0;
		UINT64 inChannelLayout = 0, outChannelLayout = 0;
		int bytesPerSample = 0;
		bool needsUpdateResampler = false;
		bool useResampler = false;
	};
}

