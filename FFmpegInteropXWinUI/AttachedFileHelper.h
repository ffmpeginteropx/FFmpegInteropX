#pragma once

#include "pch.h"

extern "C"
{
#include <libavformat\avformat.h>
}
#include <ppltasks.h>
#include "AttachedFile.h"

namespace FFmpegInteropX
{
	using namespace winrt::Windows::Foundation;
	using namespace winrt::Windows::Storage;
	using namespace winrt::Windows::Storage::Streams;

	class AttachedFileHelper sealed
	{
	public:
		virtual ~AttachedFileHelper()
		{
			if (extractedFiles.Size() > 0)
			{
				CleanupTempFiles(config.AttachmentCacheFolderName(), InstanceId());
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
			attachedFiles.Append(file);
		}

		winrt::Windows::Foundation::IAsyncOperation<winrt::Windows::Storage::StorageFile> ExtractFileAsync(winrt::FFmpegInteropXWinUI::AttachedFile attachment)
		{
			StorageFile file = { nullptr };
			auto result = extractedFiles.Lookup(attachment.Name());
			if (result != nullptr)
			{
				file = result;
			}
			else
			{
				if (this->instanceId.empty())
				{
					GUID gdn;
					CoCreateGuid(&gdn);
					instanceId = winrt::to_hstring(winrt::guid(gdn));
				}

				auto folder = co_await ApplicationData::Current().TemporaryFolder().CreateFolderAsync(
					config.AttachmentCacheFolderName(), CreationCollisionOption::OpenIfExists);
				auto instanceFolder = co_await folder.CreateFolderAsync(instanceId, CreationCollisionOption::OpenIfExists);
				file = (co_await instanceFolder.CreateFileAsync(attachment.Name(), CreationCollisionOption::ReplaceExisting));
				co_await FileIO::WriteBufferAsync(file, attachment.as<winrt::FFmpegInteropXWinUI::implementation::AttachedFile>()->GetBuffer());

				extractedFiles.Insert(attachment.Name(), file);
			}
			co_return file;
		};

		static winrt::Windows::Foundation::IAsyncAction CleanupTempFiles(winrt::hstring folderName, winrt::hstring instanceId)
		{
			try
			{
				auto folder = co_await ApplicationData::Current().TemporaryFolder().GetFolderAsync(folderName);
				auto instancefolder = co_await folder.GetFolderAsync(instanceId);
				auto files = co_await instancefolder.GetFilesAsync();
				for (auto& file : files)
				{
					co_await file.DeleteAsync();
				}
				co_await instancefolder.DeleteAsync();
			}
			catch (...)
			{
				OutputDebugString(L"Failed to cleanup temp files.");
			}
		}

	private:
		winrt::Windows::Foundation::Collections::IMap<winrt::hstring, winrt::Windows::Storage::StorageFile> extractedFiles{ winrt::single_threaded_map<winrt::hstring, winrt::Windows::Storage::StorageFile>() };
		winrt::Windows::Foundation::Collections::IVector<winrt::FFmpegInteropXWinUI::AttachedFile> attachedFiles{ winrt::single_threaded_vector<winrt::FFmpegInteropXWinUI::AttachedFile>() };
		winrt::FFmpegInteropXWinUI::MediaSourceConfig config;
		winrt::hstring instanceId;
	};

}