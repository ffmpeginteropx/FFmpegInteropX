#pragma once

//#include <wrl.h>
//#include <wrl/implements.h>
//#include <windows.storage.streams.h>
//#include <robuffer.h>
//#include <vector>

namespace NativeBuffer
{
	struct __declspec(uuid("905a0fef-bc53-11df-8c49-001e4fc686da")) IBufferByteAccess : ::IUnknown
	{
		virtual HRESULT __stdcall Buffer(uint8_t** value) = 0;
	};

	struct NativeBuffer : winrt::implements<NativeBuffer, winrt::Windows::Storage::Streams::IBuffer, IBufferByteAccess>
	{
		//InspectableClass("NativeBuffer.NativeBuffer", BaseTrust)

	public:
		virtual ~NativeBuffer()
		{
			if (m_free)
			{
				m_free(m_opaque);
			}
			m_pObject = nullptr;
		}


		NativeBuffer(byte* buffer, UINT32 totalSize)
		{
			m_length = totalSize;
			m_buffer = buffer;
			m_free = NULL;
			m_opaque = NULL;
			m_pObject = nullptr;

			//return S_OK;
		}

		NativeBuffer(byte* buffer, UINT32 totalSize, void(*free)(void* opaque), void* opaque)
		{
			m_length = totalSize;
			m_buffer = buffer;
			m_free = free;
			m_opaque = opaque;
			m_pObject = nullptr;

			//return S_OK;
		}

		NativeBuffer(byte* buffer, UINT32 totalSize, winrt::Windows::Foundation::IInspectable pObject)
		{
			m_length = totalSize;
			m_buffer = buffer;
			m_free = NULL;
			m_opaque = NULL;
			m_pObject = pObject;

			//return S_OK;
		}

		HRESULT __stdcall Buffer(uint8_t** value) final
		{
			*value = m_buffer;
			return S_OK;
		}

		uint32_t Capacity() const { return m_length; }
		uint32_t Length() const { return m_length; }
		void Length(uint32_t length) { m_length = length; }


	private:
		UINT32 m_length = 0;
		byte* m_buffer = NULL;
		void(*m_free)(void* opaque);
		void* m_opaque;
		winrt::Windows::Foundation::IInspectable m_pObject = { nullptr };
	};
}