#pragma once
extern "C"
{
#include <libavformat/avformat.h>
}
namespace FFmpegInterop
{
	ref class IAvEffect abstract
	{
	public:
		virtual	~IAvEffect() {}

	internal:
		
		/// <summary>
		/// Adds a frame to the filter graph. If the frame is a hw frame, the graph may copy that frame 
		/// to a software frame, in which case outswFrame will contain the pointer to that frame
		/// </summary>
		/// <param name="frame">The input frame</param>
		/// <param name="outswFrame">Pointer to software frame copy, in case the filter had to transfer data from input hw frame</param>
		/// <returns></returns>
		virtual HRESULT AddFrame(AVFrame* frame, AVFrame* outswFrame) abstract;
		/// <summary>
		/// Gets a frame from the filter graph
		/// </summary>
		/// <param name="frame"></param>
		/// <returns></returns>
		virtual HRESULT GetFrame(AVFrame* frame) abstract;
	};
}
