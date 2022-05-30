#pragma once

namespace NativeBuffer
{
	class NativeBufferFactory
	{
	public:
		static Windows::Storage::Streams::IBuffer^ CreateNativeBuffer(UINT32 nNumberOfBytes);
		static Windows::Storage::Streams::IBuffer^ CreateNativeBuffer(LPVOID lpBuffer, UINT32 nNumberOfBytes);
		static Windows::Storage::Streams::IBuffer^ CreateNativeBuffer(LPVOID lpBuffer, UINT32 nNumberOfBytes, void(*free)(void* opaque), void* opaque);
		static Windows::Storage::Streams::IBuffer^ CreateNativeBuffer(LPVOID lpBuffer, UINT32 nNumberOfBytes, Platform::Object^ pObject);
	};
}