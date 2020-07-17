#pragma once
#include <mfapi.h>
#include <mftransform.h>
#include <Mferror.h>

namespace FFmpegInterop
{
	ref class MFTEnumerator {
	internal:
		HRESULT FindVideoDecoder(
			const GUID& subtype,
			BOOL bAllowAsync,
			BOOL bAllowHardware,
			BOOL bAllowTranscode,
			IMFTransform** ppDecoder2
		)
		{
			HRESULT hr = S_OK;
			UINT32 count = 0;

			IMFActivate** ppActivate = NULL;

			MFT_REGISTER_TYPE_INFO info = { MFMediaType_Video,  MFVideoFormat_H264_ES };
			MFT_REGISTER_TYPE_INFO output = { MFMediaType_Video,  MFVideoFormat_ARGB32 };
			
			UINT32 unFlags = MFT_ENUM_FLAG_SYNCMFT | MFT_ENUM_FLAG_LOCALMFT |
				MFT_ENUM_FLAG_SORTANDFILTER;

			if (bAllowAsync)
			{
				unFlags |= MFT_ENUM_FLAG_ASYNCMFT;
			}
			if (bAllowHardware)
			{
				unFlags |= MFT_ENUM_FLAG_HARDWARE;
			}
			if (bAllowTranscode)
			{
				unFlags |= MFT_ENUM_FLAG_TRANSCODE_ONLY;
			}

			hr = MFTEnumEx(MFT_CATEGORY_VIDEO_DECODER,
				MFT_ENUM_FLAG_ALL,
				NULL,      // Input type
				NULL,       // Output type
				&ppActivate,
				&count);

			if (SUCCEEDED(hr) && count == 0)
			{
				hr = E_FAIL;
			}



			if (SUCCEEDED(hr))
			{
				for (UINT32 i = 0; i < count; i++)
				{
					IMFTransform* ppDecoder;
					hr = ppActivate[i]->ActivateObject(IID_PPV_ARGS(&ppDecoder));
					if (!ppDecoder) continue;
					IMFAttributes* atributes;
					ppDecoder->GetAttributes(&atributes);
					if (!atributes) continue;
					uint32 async;
					atributes->GetUINT32(MF_TRANSFORM_ASYNC, &async);
					atributes->SetUINT32(MF_TRANSFORM_ASYNC_UNLOCK, 1);

					DWORD bla;
					IMFMediaType* mediaType;
					hr = MFCreateMediaType(&mediaType);
					hr = mediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
					hr = mediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264);
					
					hr = ppDecoder->SetInputType(0, mediaType, 0);
					//hr = ppDecoder->

					if (SUCCEEDED(hr)) {
						IMFMediaType* mediaType2 = NULL;
						/*hr = MFCreateMediaType(&mediaType2);
						hr = mediaType2->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
						hr = mediaType2->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_ARGB32);
						hr = ppDecoder->SetOutputType(0, mediaType2, 0);*/
						int index = 0;
						while (ppDecoder->GetInputAvailableType(0, index, &mediaType2) != MF_E_NO_MORE_TYPES)
						{
							GUID subtype;

							mediaType2->GetGUID(MF_MT_SUBTYPE, &subtype);
							mediaType2->GetGUID(MF_MT_SUBTYPE, &subtype);
							index++;
						}
					}
					ppActivate[i]->Release();


				}
				CoTaskMemFree(ppActivate);
			}
			return hr;
		}
	};
}