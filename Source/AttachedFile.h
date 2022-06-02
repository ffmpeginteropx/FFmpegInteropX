#pragma once

extern "C"
{
#include <libavformat\avformat.h>
}
#include "AttachedFile.g.h"
#include <NativeBufferFactory.h>


using namespace winrt::Windows::Storage;
using namespace winrt::Windows::Storage::Streams;
using namespace NativeBuffer;

namespace winrt::FFmpegInteropX::implementation
{
	struct AttachedFile: AttachedFileT<AttachedFile>
	{
	public:
		 hstring Name() { return name; } 
		 hstring MimeType() { return mimeType; } 
		 DWORD Size() { return stream->codecpar->extradata_size; }

		AttachedFile(hstring name, hstring mimeType, AVStream* stream)
		{
			this->name = name;
			this->mimeType = mimeType;
			this->stream = stream;
		}

		IBuffer GetBuffer()
		{
			return NativeBufferFactory::CreateNativeBuffer(stream->codecpar->extradata, (DWORD)Size);
		}

	private:
		hstring name;
		hstring mimeType;

		AVStream* stream;
	};

}

namespace winrt::FFmpegInteropX::factory_implementation
{
	struct AttachedFile : AttachedFileT<AttachedFile, implementation::AttachedFile>
	{
	};
}