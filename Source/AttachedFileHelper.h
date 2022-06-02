#pragma once

#include <winrt/Windows.Foundation.Collections.h>
#include <pplawait.h>
#include <winrt/FFmpegInteropX.h>

extern "C"
{
#include <libavformat\avformat.h>
}

#include "AttachedFile.h"


using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::Storage::Streams;
using namespace winrt::FFmpegInteropX::implementation;
using namespace Concurrency;

namespace FFmpegInteropX
{
	class AttachedFileHelper sealed
	{
	public:
		virtual ~AttachedFileHelper()
		{
			if (extractedFiles.size() > 0)
			{
				CleanupTempFiles(config->AttachmentCacheFolderName, InstanceId());
			}
		}

		AttachedFileHelper(MediaSourceConfig config)
		{
			this->config = config;
		}

		Vector<AttachedFile> AttachedFiles() { return attachedFiles; }
		String InstanceId() { return instanceId; }

		void AddAttachedFile(AttachedFile file)
		{
			attachedFiles->Append(file);
		}

		task<StorageFile> ExtractFileAsync(AttachedFile attachment)
		{
			StorageFile file;
			auto result = extractedFiles.find(attachment->Name);
			if (result != extractedFiles.end())
			{
				file = result->second;
			}
			else
			{
				if (this->instanceId == nullptr)
				{
					GUID gdn;
					CoCreateGuid(&gdn);
					instanceId = Guid(gdn).ToString();
				}

				auto folder = co_await ApplicationData::Current->TemporaryFolder->CreateFolderAsync(
					config->AttachmentCacheFolderName, CreationCollisionOption::OpenIfExists);
				auto instanceFolder = co_await folder->CreateFolderAsync(instanceId, CreationCollisionOption::OpenIfExists);
				file = co_await instanceFolder->CreateFileAsync(attachment->Name, CreationCollisionOption::ReplaceExisting);
				co_await FileIO::WriteBufferAsync(file, attachment->GetBuffer());

				extractedFiles[attachment->Name] = file;
			}
			co_return file;
		};

		static task<void> CleanupTempFiles(String folderName, String instanceId)
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
		std::map<String, StorageFile> extractedFiles;
		Vector<AttachedFile> attachedFiles = ref new Vector<AttachedFile>();
		MediaSourceConfig config;
		String instanceId;
	};

}