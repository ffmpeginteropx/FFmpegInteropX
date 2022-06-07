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
#include "UncompressedSampleProvider.h"

#pragma region Desktop Family
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP) && !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

// {47537213-8cfb-4722-aa34-fbc9e24d77b8}   MF_MT_CUSTOM_VIDEO_PRIMARIES    {BLOB (MT_CUSTOM_VIDEO_PRIMARIES)}
DEFINE_GUID(MF_MT_CUSTOM_VIDEO_PRIMARIES,
	0x47537213, 0x8cfb, 0x4722, 0xaa, 0x34, 0xfb, 0xc9, 0xe2, 0x4d, 0x77, 0xb8);

typedef struct _MT_CUSTOM_VIDEO_PRIMARIES {
	float fRx;
	float fRy;
	float fGx;
	float fGy;
	float fBx;
	float fBy;
	float fWx;
	float fWy;
} MT_CUSTOM_VIDEO_PRIMARIES;

#endif /* WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP | WINAPI_PARTITION_GAMES) */
#pragma endregion

namespace FFmpegInteropX
{
	

	class UncompressedVideoSampleProvider : public UncompressedSampleProvider
	{
	public:
		virtual ~UncompressedVideoSampleProvider();
		virtual void Flush() override
		{
			hasFirstInterlacedFrame = false;
			UncompressedSampleProvider::Flush();
		}

		UncompressedVideoSampleProvider(
			std::shared_ptr<FFmpegReader> reader,
			AVFormatContext* avFormatCtx,
			AVCodecContext* avCodecCtx,
			MediaSourceConfig const& config,
			int streamIndex,
			HardwareDecoderStatus hardwareDecoderStatus);
		winrt::Windows::Media::Core::IMediaStreamDescriptor CreateStreamDescriptor() override;
		virtual HRESULT CreateBufferFromFrame(winrt::Windows::Storage::Streams::IBuffer* pBuffer, winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DSurface* surface, AVFrame* avFrame, int64_t& framePts, int64_t& frameDuration) override;
		virtual HRESULT SetSampleProperties(winrt::Windows::Media::Core::MediaStreamSample sample) override;
		void ReadFrameProperties(AVFrame* avFrame, int64_t& framePts);
		void CheckFrameSize(AVFrame* avFrame);
		AVPixelFormat GetOutputPixelFormat() { return m_OutputPixelFormat; }
		int TargetWidth;
		int TargetHeight;
		BYTE* TargetBuffer;

		virtual void NotifyCreateSource() override
		{
			if (VideoInfo().FramesPerSecondOverride() > 0 && VideoInfo().FramesPerSecond() > 0)
			{
				timeBaseFactor *= VideoInfo().FramesPerSecond() / VideoInfo().FramesPerSecondOverride();
			}

			UncompressedSampleProvider::NotifyCreateSource();
		}

	private:
		void SelectOutputFormat();
		HRESULT InitializeScalerIfRequired(AVFrame* avFrame);
		HRESULT FillLinesAndBuffer(int* linesize, BYTE** data, AVBufferRef** buffer, int width, int height, bool isSourceBuffer);
		AVBufferRef* AllocateBuffer(int requestedSize, AVBufferPool** bufferPool, int* bufferPoolSize);
		static int get_buffer2(AVCodecContext* avCodecContext, AVFrame* frame, int flags);

		winrt::hstring outputMediaSubtype;
		int outputWidth;
		int outputHeight;
		int outputFrameHeight;
		int outputFrameWidth;
		bool outputDirectBuffer;

		AVBufferPool* sourceBufferPool;
		int sourceBufferPoolSize;
		AVBufferPool* targetBufferPool;
		int targetBufferPoolSize;
		AVPixelFormat m_OutputPixelFormat;
		SwsContext* m_pSwsCtx;
		bool m_interlaced_frame;
		bool m_top_field_first;
		AVChromaLocation m_chroma_location;
		bool hasFirstInterlacedFrame;
		winrt::Windows::Foundation::IInspectable maxCLL;
		winrt::Windows::Foundation::IInspectable maxFALL;
		winrt::Windows::Foundation::IInspectable minLuminance;
		winrt::Windows::Foundation::IInspectable maxLuminance;
		winrt::Windows::Foundation::IInspectable customPrimaries;
	};
}

