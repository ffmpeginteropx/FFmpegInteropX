#pragma once
extern "C"
{
#include <libavformat/avformat.h>
}
namespace FFmpegInteropX
{
	class IAvEffect abstract
	{
	public:
		virtual	~IAvEffect() {}
		
		virtual HRESULT AddFrame(AVFrame* frame) abstract;
	
		virtual HRESULT GetFrame(AVFrame* frame) abstract;
	};
}
