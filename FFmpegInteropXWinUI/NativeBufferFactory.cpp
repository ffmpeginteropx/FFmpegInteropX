#include "pch.h"

#include "NativeBuffer.h"
#include "NativeBufferFactory.h"
#include "winrt/Windows.Storage.Streams.h"

using namespace NativeBuffer;

winrt::Windows::Storage::Streams::IBuffer* NativeBufferFactory::CreateNativeBuffer(UINT32 nNumberOfBytes)
{
	auto lpBuffer = (byte*)malloc(nNumberOfBytes);
	return CreateNativeBuffer(lpBuffer, nNumberOfBytes, &free, lpBuffer);
}

winrt::Windows::Storage::Streams::IBuffer* NativeBufferFactory::CreateNativeBuffer(LPVOID lpBuffer, UINT32 nNumberOfBytes)
{
	return CreateNativeBuffer(lpBuffer, nNumberOfBytes, NULL, NULL);
}

winrt::Windows::Storage::Streams::IBuffer* NativeBufferFactory::CreateNativeBuffer(LPVOID lpBuffer, UINT32 nNumberOfBytes, void(*free)(void *opaque), void *opaque)
{
	winrt::com_ptr<NativeBuffer> nativeBuffer;
	winrt::make<NativeBuffer>(&nativeBuffer, (byte*)lpBuffer, nNumberOfBytes, free, opaque);
	auto iinspectable = (IInspectable *)reinterpret_cast<IInspectable *>(nativeBuffer.get());
	winrt::Windows::Storage::Streams::IBuffer* buffer = reinterpret_cast<winrt::Windows::Storage::Streams::IBuffer*>(iinspectable);

	return buffer;
}

winrt::Windows::Storage::Streams::IBuffer* NativeBufferFactory::CreateNativeBuffer(LPVOID lpBuffer, UINT32 nNumberOfBytes, const IInspectable &pObject)
{
	winrt::com_ptr<NativeBuffer> nativeBuffer;
	winrt::make<NativeBuffer>(&nativeBuffer, (byte *)lpBuffer, nNumberOfBytes, pObject);
	auto iinspectable = (IInspectable *)reinterpret_cast<IInspectable *>(nativeBuffer.get());
	winrt::Windows::Storage::Streams::IBuffer* buffer = reinterpret_cast<winrt::Windows::Storage::Streams::IBuffer*>(iinspectable);

	return buffer;
}