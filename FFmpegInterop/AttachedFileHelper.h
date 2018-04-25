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

		AttachedFileHelper(FFmpegInteropConfig^ config)
		{
			this->config = config;
		}

		void AddAttachedFile(AttachedFile^ file)
		{
			attachedFiles.push_back(file);
			files[file->Name] = file;
		}

		void EnsureIsValidFile(AttachedFile^ file)
		{
			if (files.find(file->Name) == files.end() || files[file->Name] != file)
			{
				throw ref new InvalidArgumentException("The attachment is not part of this ffmpeg instance.");
			}
		}

		IAsyncOperation<bool>^ EnsureFileExtracted(String^ fileName)
		{
			if (files.find(fileName) == files.end())
			{
				return create_async([] { return false; });
			}
			else if (extractedFiles.find(fileName) != extractedFiles.end())
			{
				return create_async([] { return true; });
			}
			else
			{
				AttachedFile^ attachment = files.at(fileName);
				return create_async([this, fileName, attachment]
				{
					return create_task(ApplicationData::Current->TemporaryFolder->CreateFolderAsync(
						config->AttachmentCacheFolderName, CreationCollisionOption::OpenIfExists)).then([this, fileName, attachment](task<StorageFolder^> t)
					{
						auto folder = t.get();
						return create_task(folder->CreateFileAsync(
							fileName, CreationCollisionOption::ReplaceExisting)).then([this, fileName, attachment](task<StorageFile^> t)
						{
							auto file = t.get();
							return create_task(FileIO::WriteBufferAsync(file, attachment->GetBuffer())).then([this, fileName, attachment](task<void> t)
							{
								t.wait();
								extractedFiles[fileName] = attachment;
								return true;
							});
						});
					});
				});
			}
		};

	private:
		std::map<String^, AttachedFile^> files;
		std::map<String^, AttachedFile^> extractedFiles;
		std::vector<AttachedFile^> attachedFiles;
		FFmpegInteropConfig^ config;
	};

}