#pragma once
#include "pch.h"

namespace NativeBuffer
{
	class NativeBufferFactory
	{
	public:
		static winrt::Windows::Storage::Streams::IBuffer* CreateNativeBuffer(UINT32 nNumberOfBytes);
		static winrt::Windows::Storage::Streams::IBuffer* CreateNativeBuffer(LPVOID lpBuffer, UINT32 nNumberOfBytes);
		static winrt::Windows::Storage::Streams::IBuffer* CreateNativeBuffer(LPVOID lpBuffer, UINT32 nNumberOfBytes, void(*free)(void* opaque), void* opaque);
		static winrt::Windows::Storage::Streams::IBuffer* CreateNativeBuffer(LPVOID lpBuffer, UINT32 nNumberOfBytes, const winrt::Windows::Foundation::IInspectable &pObject);
	};
}