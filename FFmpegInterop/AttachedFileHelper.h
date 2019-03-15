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
	public:
		virtual ~AttachedFileHelper()
		{
			if (extractedFiles.size() > 0)
			{
				CleanupTempFiles(config->AttachmentCacheFolderName, InstanceId);
			}
		}

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
		
		task<StorageFile^> ExtractFileAsync(AttachedFile^ attachment)
		{
			StorageFile^ file;
			auto result = extractedFiles.find(attachment->Name);
			if (result != extractedFiles.end())
			{
				file = result->second;
			}
			else
			{
				auto folder = co_await ApplicationData::Current->TemporaryFolder->CreateFolderAsync(
					config->AttachmentCacheFolderName, CreationCollisionOption::OpenIfExists);
				auto instanceFolder = co_await folder->CreateFolderAsync(instanceId, CreationCollisionOption::OpenIfExists);
				file = co_await instanceFolder->CreateFileAsync(attachment->Name, CreationCollisionOption::ReplaceExisting);
				co_await FileIO::WriteBufferAsync(file, attachment->GetBuffer());

				extractedFiles[attachment->Name] = file;
			}
			return file;
		};

		static task<void> CleanupTempFiles(String^ folderName, String^ instanceId)
		{
			try
			{
				auto folder = co_await ApplicationData::Current->TemporaryFolder->GetFolderAsync(folderName);
				auto instancefolder = co_await folder->GetFolderAsync(instanceId);
				auto files = co_await instancefolder->GetFilesAsync();
				for each (auto file in files)
				{
					co_await file->DeleteAsync();
				}
				co_await instancefolder->DeleteAsync();
			}
			catch (...)
			{
				OutputDebugString(L"Failed to cleanup temp files.");
			}
		}

	private:
		std::map<String^, StorageFile^> extractedFiles;
		Vector<AttachedFile^>^ attachedFiles = ref new Vector<AttachedFile^>();
		FFmpegInteropConfig^ config;
		String^ instanceId;
	};

}