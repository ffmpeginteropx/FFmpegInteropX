#include "pch.h"

#include "NativeBuffer.h"
#include "NativeBufferFactory.h"
#include "winrt/Windows.Storage.Streams.h"

using namespace NativeBuffer;

Windows::Storage::Streams::IBuffer NativeBufferFactory::CreateNativeBuffer(UINT32 nNumberOfBytes)
{
	auto lpBuffer = (byte*)malloc(nNumberOfBytes);
	return CreateNativeBuffer(lpBuffer, nNumberOfBytes, &free, lpBuffer);
}

Windows::Storage::Streams::IBuffer NativeBufferFactory::CreateNativeBuffer(LPVOID lpBuffer, UINT32 nNumberOfBytes)
{
	return CreateNativeBuffer(lpBuffer, nNumberOfBytes, NULL, NULL);
}

Windows::Storage::Streams::IBuffer NativeBufferFactory::CreateNativeBuffer(LPVOID lpBuffer, UINT32 nNumberOfBytes, void(*free)(void *opaque), void *opaque)
{
	winrt::com_ptr<NativeBuffer> nativeBuffer;
	winrt::make<NativeBuffer>(&nativeBuffer, (byte*)lpBuffer, nNumberOfBytes, free, opaque);
	auto iinspectable = (IInspectable *)reinterpret_cast<IInspectable *>(nativeBuffer.get());
	Windows::Storage::Streams::IBuffer buffer = reinterpret_cast<Windows::Storage::Streams::IBuffer >(iinspectable);

	return buffer;
}

Windows::Storage::Streams::IBuffer NativeBufferFactory::CreateNativeBuffer(LPVOID lpBuffer, UINT32 nNumberOfBytes, Platform::Object pObject)
{
	winrt::com_ptr<NativeBuffer> nativeBuffer;
	winrt::make<NativeBuffer>(&nativeBuffer, (byte *)lpBuffer, nNumberOfBytes, pObject);
	auto iinspectable = (IInspectable *)reinterpret_cast<IInspectable *>(nativeBuffer.Get());
	Windows::Storage::Streams::IBuffer buffer = reinterpret_cast<Windows::Storage::Streams::IBuffer >(iinspectable);

	return buffer;
}