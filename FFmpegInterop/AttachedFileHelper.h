#pragma once

#include <collection.h>
#include <pplawait.h>
#include "libavformat\avformat.h"

#include "AttachedFile.h"

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;

using namespace Concurrency;

namespace FFmpegInterop
{
	ref class AttachedFileHelper sealed
	{
	internal:

		AttachedFileHelper(FFmpegInteropConfig^ config)
		{
			this->config = config;
		}

		property Vector<AttachedFile^>^ AttachedFiles { Vector<AttachedFile^>^ get() { return attachedFiles; } }
		property String^ InstanceId { String^ get() { return instanceId; } }

		void AddAttachedFile(AttachedFile^ file)
		{
			attachedFiles->Append(file);
		}

		IAsyncOperation<StorageFile^>^ ExtractFile(AttachedFile^ attachment)
		{
			auto result = extractedFiles.find(attachment->Name);
			if (result != extractedFiles.end())
			{
				return create_async([result] { return result->second; });
			}
			else
			{
				return create_async([this, attachment]
				{
					return create_task(ApplicationData::Current->TemporaryFolder->CreateFolderAsync(
						config->AttachmentCacheFolderName, CreationCollisionOption::OpenIfExists)).then([this, attachment](task<StorageFolder^> t)
					{
						auto folder = t.get();
						if (this->instanceId == nullptr)
						{
							GUID gdn;
							CoCreateGuid(&gdn);
							Guid gd(gdn);
							instanceId = gd.ToString();
						}
						return create_task(folder->CreateFolderAsync(
							instanceId, CreationCollisionOption::OpenIfExists)).then([this, attachment](task<StorageFolder^> t)
						{
							auto instanceFolder = t.get();
							return create_task(instanceFolder->CreateFileAsync(
								attachment->Name, CreationCollisionOption::ReplaceExisting)).then([this, attachment](task<StorageFile^> t)
							{
								auto file = t.get();
								return create_task(FileIO::WriteBufferAsync(file, attachment->GetBuffer())).then([this, file, attachment](task<void> t)
								{
									t.wait();
									extractedFiles[attachment->Name] = file;
									return file;
								});
							});
						});
					});
				});
			}
		};

	private:
		std::map<String^, StorageFile^> extractedFiles;
		Vector<AttachedFile^>^ attachedFiles = ref new Vector<AttachedFile^>();
		FFmpegInteropConfig^ config;
		String^ instanceId;
	};

}