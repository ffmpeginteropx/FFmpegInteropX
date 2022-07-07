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

    class AttachedFileHelper
    {
    public:
        virtual ~AttachedFileHelper()
        {
            if (extractedFiles.Size() > 0)
            {
                CleanupTempFiles(config.AttachmentCacheFolderName(), InstanceId());
            }
        }

        AttachedFileHelper(winrt::FFmpegInteropX::MediaSourceConfig const& config)
        {
            this->config = config;
        }

        winrt::Windows::Foundation::Collections::IVector<winrt::FFmpegInteropX::AttachedFile> AttachedFiles() { return attachedFiles; }
        winrt::hstring InstanceId() { return instanceId; }

        void AddAttachedFile(winrt::FFmpegInteropX::AttachedFile const& file)
        {
            attachedFiles.Append(file);
        }

        winrt::Windows::Foundation::IAsyncOperation<winrt::Windows::Storage::StorageFile> ExtractFileAsync(winrt::FFmpegInteropX::AttachedFile attachment)
        {
            StorageFile file = { nullptr };
            auto result = extractedFiles.TryLookup(attachment.Name());
            if (result != nullptr)
            {
                file = result;
            }
            else
            {
                if (this->instanceId.empty())
                {
                    GUID gdn;
                    auto hr = CoCreateGuid(&gdn);
                    if (FAILED(hr))
                    {
                        winrt::throw_hresult(hr);
                    }
                    instanceId = winrt::to_hstring(winrt::guid(gdn));
                }

                auto folder = co_await ApplicationData::Current().TemporaryFolder().CreateFolderAsync(
                    config.AttachmentCacheFolderName(), CreationCollisionOption::OpenIfExists);
                auto instanceFolder = co_await folder.CreateFolderAsync(instanceId, CreationCollisionOption::OpenIfExists);
                file = (co_await instanceFolder.CreateFileAsync(attachment.Name(), CreationCollisionOption::ReplaceExisting));
                co_await FileIO::WriteBufferAsync(file, attachment.as<winrt::FFmpegInteropX::implementation::AttachedFile>()->GetBuffer());

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
                for (auto file : files)
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
        winrt::Windows::Foundation::Collections::IVector<winrt::FFmpegInteropX::AttachedFile> attachedFiles{ winrt::single_threaded_vector<winrt::FFmpegInteropX::AttachedFile>() };
        winrt::FFmpegInteropX::MediaSourceConfig config = { nullptr };
        winrt::hstring instanceId{};
    };

}
