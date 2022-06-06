#pragma once

#include "pch.h"

extern "C"
{
#include <libavformat\avformat.h>
}

#include "AttachedFile.h"

using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::Storage::Streams;

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

		AttachedFileHelper(winrt::FFmpegInteropXWinUI::MediaSourceConfig const& config)
		{
			this->config = config;
		}

		winrt::Windows::Foundation::Collections::IVector<winrt::FFmpegInteropXWinUI::AttachedFile> AttachedFiles() { return attachedFiles; }
		winrt::hstring InstanceId() { return instanceId; }

		void AddAttachedFile(winrt::FFmpegInteropXWinUI::AttachedFile file)
		{
			attachedFiles->Append(file);
		}

		concurrency::task<StorageFile> ExtractFileAsync(winrt::FFmpegInteropXWinUI::AttachedFile attachment)
		{
			StorageFile file;
			auto result = extractedFiles.find(attachment.Name());
			if (result != extractedFiles.end())
			{
				file = result->second;
			}
			else
			{
				if (this->instanceId.empty())
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

		static task<void> CleanupTempFiles(winrt::hstring folderName, winrt::hstring instanceId)
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
		std::map<winrt::hstring, StorageFile> extractedFiles;
		Vector<AttachedFile> attachedFiles = ref new Vector<AttachedFile>();
		MediaSourceConfig config;
		winrt::hstring instanceId;
	};

}