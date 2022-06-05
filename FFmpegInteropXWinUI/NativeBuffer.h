#pragma once

#include <pch.h>

namespace NativeBuffer
{
	class NativeBuffer :
		winrt::Windows::Storage::Streams::IBuffer,
		Windows::Storage::Streams::IBufferByteAccess
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

		STDMETHODIMP Buffer(byte **value)
		{
			*value = m_buffer;

			return S_OK;
		}

		STDMETHODIMP get_Capacity(UINT32 *value)
		{
			*value = m_length;

			return S_OK;
		}

		STDMETHODIMP get_Length(UINT32 *value)
		{
			*value = m_length;

			return S_OK;
		}

		STDMETHODIMP put_Length(UINT32 value)
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