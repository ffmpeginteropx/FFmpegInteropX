#include "pch.h"
#include "NativeBuffer.h"
#include "NativeBufferFactory.h"

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

winrt::Windows::Storage::Streams::IBuffer* NativeBufferFactory::CreateNativeBuffer(LPVOID lpBuffer, UINT32 nNumberOfBytes, void(*free)(void* opaque), void* opaque)
{
	/*winrt::com_ptr<NativeBuffer> nativeBuffer;
	winrt::make<NativeBuffer>(&nativeBuffer, (byte*)lpBuffer, nNumberOfBytes, free, opaque);
	auto iinspectable = nativeBuffer.as<winrt::Windows::Foundation::IInspectable>();
	winrt::Windows::Storage::Streams::IBuffer* buffer = iinspectable.as< winrt::Windows::Storage::Streams::IBuffer*>();
	*/
	return nullptr;
}

winrt::Windows::Storage::Streams::IBuffer* NativeBufferFactory::CreateNativeBuffer(LPVOID lpBuffer, UINT32 nNumberOfBytes, const winrt::Windows::Foundation::IInspectable& pObject)
{
	//winrt::com_ptr<NativeBuffer> nativeBuffer;
	//winrt::make<NativeBuffer>(&nativeBuffer, (byte*)lpBuffer, nNumberOfBytes, pObject);
	//auto iinspectable = nativeBuffer.as<winrt::Windows::Foundation::IInspectable>();
	//winrt::Windows::Storage::Streams::IBuffer* buffer = iinspectable.as< winrt::Windows::Storage::Streams::IBuffer*>();

	return nullptr;
}