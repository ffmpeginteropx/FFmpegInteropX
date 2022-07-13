#include "pch.h"
#include "NativeBuffer.h"
#include "NativeBufferFactory.h"

using namespace NativeBuffer;

winrt::Windows::Storage::Streams::IBuffer NativeBufferFactory::CreateNativeBuffer(UINT32 nNumberOfBytes)
{
    auto lpBuffer = (byte*)malloc(nNumberOfBytes);
    return CreateNativeBuffer(lpBuffer, nNumberOfBytes, &free, lpBuffer);
}

winrt::Windows::Storage::Streams::IBuffer NativeBufferFactory::CreateNativeBuffer(LPVOID lpBuffer, UINT32 nNumberOfBytes)
{
    return CreateNativeBuffer(lpBuffer, nNumberOfBytes, NULL, NULL);
}

winrt::Windows::Storage::Streams::IBuffer NativeBufferFactory::CreateNativeBuffer(LPVOID lpBuffer, UINT32 nNumberOfBytes, void(*free)(void* opaque), void* opaque)
{    
    auto buffer = winrt::make<NativeBuffer>((byte*)lpBuffer, nNumberOfBytes, free, opaque);

    return buffer;
}

winrt::Windows::Storage::Streams::IBuffer NativeBufferFactory::CreateNativeBuffer(LPVOID lpBuffer, UINT32 nNumberOfBytes, const winrt::Windows::Foundation::IInspectable& pObject)
{
    auto buffer = winrt::make<NativeBuffer>((byte*)lpBuffer, nNumberOfBytes, pObject);

    return buffer;
}
