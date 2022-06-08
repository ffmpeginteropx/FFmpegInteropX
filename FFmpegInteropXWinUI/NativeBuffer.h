#pragma once

#include <pch.h>
//#include <wrl.h>
//#include <wrl/implements.h>

namespace NativeBuffer
{
	struct __declspec(uuid("905a0fef-bc53-11df-8c49-001e4fc686da")) IBufferByteAccess : ::IUnknown
	{
		virtual HRESULT __stdcall Buffer(uint8_t** value) = 0;
	};

	class NativeBuffer :
		//public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::RuntimeClassType::WinRtClassicComMix>>,
		winrt::Windows::Storage::Streams::IBuffer,
		IBufferByteAccess
	{

	public:
		virtual ~NativeBuffer()
		{
			if (m_free)
			{
				m_free(m_opaque);
			}
			m_pObject = { nullptr };
		}


		NativeBuffer(byte *buffer, UINT32 totalSize)
		{
			m_length = totalSize;
			m_buffer = buffer;
			m_free = NULL;
			m_opaque = NULL;
			m_pObject = { nullptr };

		}

		NativeBuffer(byte *buffer, UINT32 totalSize, void(*free)(void *opaque), void *opaque)
		{
			m_length = totalSize;
			m_buffer = buffer;
			m_free = free;
			m_opaque = opaque;
			m_pObject = { nullptr };
		}

		NativeBuffer(byte *buffer, UINT32 totalSize, IInspectable pObject)
		{
			m_length = totalSize;
			m_buffer = buffer;
			m_free = NULL;
			m_opaque = NULL;
			m_pObject = pObject;
		}

		HRESULT Buffer(byte **value)
		{
			*value = m_buffer;

			return S_OK;
		}

		HRESULT get_Capacity(UINT32 *value)
		{
			*value = m_length;

			return S_OK;
		}

		HRESULT get_Length(UINT32 *value)
		{
			*value = m_length;

			return S_OK;
		}

		HRESULT put_Length(UINT32 value)
		{
			return E_FAIL;
		}


	private:
		UINT32 m_length;
		byte *m_buffer;
		void(*m_free)(void *opaque);
		void *m_opaque;
		IInspectable m_pObject {nullptr};
	};
}