//*****************************************************************************
//
//	Copyright 2015 Microsoft Corporation
//
//	Licensed under the Apache License, Version 2.0 (the "License");
//	you may not use this file except in compliance with the License.
//	You may obtain a copy of the License at
//
//	http ://www.apache.org/licenses/LICENSE-2.0
//
//	Unless required by applicable law or agreed to in writing, software
//	distributed under the License is distributed on an "AS IS" BASIS,
//	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//	See the License for the specific language governing permissions and
//	limitations under the License.
//
//*****************************************************************************

//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"

using namespace FFmpegInterop;
using namespace MediaPlayerCPP;

using namespace concurrency;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Media::Core;
using namespace Windows::Media::Playback;
using namespace Windows::Storage;
using namespace Windows::Storage::Pickers;
using namespace Windows::Storage::Streams;
using namespace Windows::UI::Popups;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

MainPage::MainPage()
{
	Config = ref new FFmpegInteropConfig();

	InitializeComponent();

	// Show the control panel on startup so user can start opening media
	Splitter->IsPaneOpen = true;

	// optionally check for recommended ffmpeg version
	FFmpegVersionInfo::CheckRecommendedVersion();

	// populate character encodings
	cbEncodings->ItemsSource = CharacterEncoding::GetCharacterEncodings();
}

void MediaPlayerCPP::MainPage::Page_Loaded(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	auto controls = Windows::Media::SystemMediaTransportControls::GetForCurrentView();
	controls->ButtonPressed += ref new Windows::Foundation::TypedEventHandler<Windows::Media::SystemMediaTransportControls ^, Windows::Media::SystemMediaTransportControlsButtonPressedEventArgs ^>(this, &MediaPlayerCPP::MainPage::OnButtonPressed);
}

void MediaPlayerCPP::MainPage::OnButtonPressed(Windows::Media::SystemMediaTransportControls ^sender, Windows::Media::SystemMediaTransportControlsButtonPressedEventArgs ^args)
{
}

void MainPage::OpenLocalFile(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	FileOpenPicker^ filePicker = ref new FileOpenPicker();
	filePicker->ViewMode = PickerViewMode::Thumbnail;
	filePicker->SuggestedStartLocation = PickerLocationId::VideosLibrary;
	filePicker->FileTypeFilter->Append("*");

	// Show file picker so user can select a file
	create_task(filePicker->PickSingleFileAsync()).then([this](StorageFile^ file)
	{
		if (file != nullptr)
		{
			currentFile = file;
			mediaElement->Stop();

			// Open StorageFile as IRandomAccessStream to be passed to FFmpegInteropMSS
			create_task(file->OpenAsync(FileAccessMode::Read)).then([this, file](task<IRandomAccessStream^> stream)
			{
				try
				{
					// Instantiate FFmpegInteropMSS using the opened local file stream
					IRandomAccessStream^ readStream = stream.get();
					create_task(FFmpegInteropMSS::CreateFromStreamAsync(readStream, Config)).then([this](FFmpegInteropMSS^ result)
					{
						FFmpegMSS = result;
						playbackItem = FFmpegMSS->CreateMediaPlaybackItem();

						// Pass MediaPlaybackItem to Media Element
						mediaElement->SetPlaybackSource(playbackItem);

						// Close control panel after file open
						Splitter->IsPaneOpen = false;

						auto controls = Windows::Media::SystemMediaTransportControls::GetForCurrentView();
						controls->IsPlayEnabled = true;
						controls->IsPauseEnabled = true;
						controls->PlaybackStatus = Windows::Media::MediaPlaybackStatus::Playing;
					});
				}
				catch (Exception^ ex)
				{
					DisplayErrorMessage(ex->Message);
				}
			});
		}
	});
}

void MainPage::URIBoxKeyUp(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e)
{
	String^ uri = safe_cast<TextBox^>(sender)->Text;

	// Only respond when the text box is not empty and after Enter key is pressed
	if (e->Key == Windows::System::VirtualKey::Enter && !uri->IsEmpty())
	{
		// Mark event as handled to prevent duplicate event to re-triggered
		e->Handled = true;

		// Set FFmpeg specific options. List of options can be found in https://www.ffmpeg.org/ffmpeg-protocols.html

		// Below are some sample options that you can set to configure RTSP streaming
		// Config->FFmpegOptions->Insert("rtsp_flags", "prefer_tcp");
		// Config->FFmpegOptions->Insert("stimeout", 100000);
		
		// Instantiate FFmpegInteropMSS using the URI
		mediaElement->Stop();
		try
		{
			create_task(FFmpegInteropMSS::CreateFromUriAsync(uri, Config)).then([this](FFmpegInteropMSS^ result)
			{
				FFmpegMSS = result;
				playbackItem = FFmpegMSS->CreateMediaPlaybackItem();

				// Pass MediaPlaybackItem to Media Element
				mediaElement->SetPlaybackSource(playbackItem);

				// Close control panel after opening media
				Splitter->IsPaneOpen = false;
			});
		}
		catch (Exception^ ex)
		{
			DisplayErrorMessage(ex->Message);
		}
	}
}

void MainPage::ExtractFrame(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if (currentFile == nullptr)
	{
		DisplayErrorMessage("Please open a video file first.");
	}
	else
	{
		// open the file that is currently playing
		create_task(currentFile->OpenAsync(FileAccessMode::Read)).then([this](IRandomAccessStream^ stream)
		{
			try
			{
				bool exactSeek = grabFrameExactSeek->IsOn;
				// extract frame using FFmpegInterop and current position
				create_task(FrameGrabber::CreateFromStreamAsync(stream)).then([this,exactSeek](FrameGrabber^ frameGrabber)
				{
					create_task(frameGrabber->ExtractVideoFrameAsync(mediaElement->Position, exactSeek)).then([this](VideoFrame^ frame)
					{
						auto filePicker = ref new FileSavePicker();
						filePicker->SuggestedStartLocation = PickerLocationId::VideosLibrary;
						filePicker->DefaultFileExtension = ".jpg";
						filePicker->FileTypeChoices->Insert("Jpeg file", ref new Platform::Collections::Vector<String^>(1, ".jpg"));

						// Show file picker so user can select a file
						create_task(filePicker->PickSaveFileAsync()).then([this, frame](StorageFile^ file)
						{
							if (file != nullptr)
							{
								create_task(file->OpenAsync(FileAccessMode::ReadWrite)).then([this, frame, file](IRandomAccessStream^ stream)
								{
									// encode frame as jpeg file
									create_task(frame->EncodeAsJpegAsync(stream)).then([this, file]
									{
										// launch file after creation
										create_task(Windows::System::Launcher::LaunchFileAsync(file)).then([this, file](bool launched)
										{
											if (!launched)
											{
												DisplayErrorMessage("File has been created:\n" + file->Path);
											}
										});
									});
								});
							}
						});
					});
				});
			}
			catch (Exception^ ex)
			{
				DisplayErrorMessage(ex->Message);
			}
		});
	}
}

void MediaPlayerCPP::MainPage::LoadSubtitleFile(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if (playbackItem == nullptr)
	{
		DisplayErrorMessage("Please open a media file before loading an external subtitle for it.");
	}
	else
	{
		FileOpenPicker^ filePicker = ref new FileOpenPicker();
		filePicker->ViewMode = PickerViewMode::Thumbnail;
		filePicker->SuggestedStartLocation = PickerLocationId::VideosLibrary;
		filePicker->FileTypeFilter->Append("*");

		// Show file picker so user can select a file
		create_task(filePicker->PickSingleFileAsync()).then([this](StorageFile^ file)
		{
			if (file != nullptr)
			{
				// Open StorageFile as IRandomAccessStream to be passed to FFmpegInteropMSS
				create_task(file->OpenAsync(FileAccessMode::Read)).then([this, file](task<IRandomAccessStream^> stream)
				{
					auto track = TimedTextSource::CreateFromStream(stream.get());
					playbackItem->Source->ExternalTimedTextSources->Append(track);
					track->Resolved += ref new Windows::Foundation::TypedEventHandler<Windows::Media::Core::TimedTextSource ^, Windows::Media::Core::TimedTextSourceResolveResultEventArgs ^>(this, &MediaPlayerCPP::MainPage::OnResolved);
				});
			}
		});
	}
}

void MediaPlayerCPP::MainPage::LoadSubtitleFileFFmpeg(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if (FFmpegMSS == nullptr)
	{
		DisplayErrorMessage("Please open a media file before loading an external subtitle for it.");
	}
	else
	{
		FileOpenPicker^ filePicker = ref new FileOpenPicker();
		filePicker->ViewMode = PickerViewMode::Thumbnail;
		filePicker->SuggestedStartLocation = PickerLocationId::VideosLibrary;
		filePicker->FileTypeFilter->Append("*");

		// Show file picker so user can select a file
		create_task(filePicker->PickSingleFileAsync()).then([this](StorageFile^ file)
		{
			if (file != nullptr)
			{
				// Open StorageFile as IRandomAccessStream to be passed to FFmpegInteropMSS
				create_task(file->OpenAsync(FileAccessMode::Read)).then([this, file](task<IRandomAccessStream^> stream)
				{
					if (playbackItem != nullptr)
					{
						timedMetadataTracksChangedToken = playbackItem->TimedMetadataTracksChanged += ref new Windows::Foundation::TypedEventHandler<Windows::Media::Playback::MediaPlaybackItem ^, Windows::Foundation::Collections::IVectorChangedEventArgs ^>(this, &MediaPlayerCPP::MainPage::OnTimedMetadataTracksChanged);
					}

					this->FFmpegMSS->AddExternalSubtitleAsync(stream.get(), file->Name);
				});
			}
		});
	}
}

void MediaPlayerCPP::MainPage::OnTimedMetadataTracksChanged(MediaPlaybackItem ^sender, Windows::Foundation::Collections::IVectorChangedEventArgs ^args)
{
	if (args->CollectionChange == Windows::Foundation::Collections::CollectionChange::ItemInserted)
	{
		sender->TimedMetadataTracksChanged -= this->timedMetadataTracksChangedToken;

		// unselect other subs
		for (unsigned int i = 0; i < sender->TimedMetadataTracks->Size; i++)
		{
			sender->TimedMetadataTracks->SetPresentationMode(i, TimedMetadataTrackPresentationMode::Disabled);
		}

		// pre-select added subtitle
		sender->TimedMetadataTracks->SetPresentationMode(args->Index, TimedMetadataTrackPresentationMode::PlatformPresented);
	}
}

void MediaPlayerCPP::MainPage::OnResolved(TimedTextSource ^sender, TimedTextSourceResolveResultEventArgs ^args)
{
	// you can rename and pre-select the loaded subtitle track(s) if you like
	if (args->Tracks->Size > 0)
	{
		auto first = args->Tracks->GetAt(0);
		first->Label = "External";
		unsigned int index;
		if (playbackItem->TimedMetadataTracks->IndexOf(first, &index))
		{
			playbackItem->TimedMetadataTracks->SetPresentationMode(index, Windows::Media::Playback::TimedMetadataTrackPresentationMode::PlatformPresented);
		}
	}
}

void MainPage::MediaFailed(Platform::Object^ sender, Windows::UI::Xaml::ExceptionRoutedEventArgs^ e)
{
	DisplayErrorMessage(e->ErrorMessage);
}

void MainPage::DisplayErrorMessage(Platform::String^ message)
{
	// Display error message
	auto errorDialog = ref new MessageDialog(message);
	errorDialog->ShowAsync();
}

void MediaPlayerCPP::MainPage::CbEncodings_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
	if (cbEncodings->SelectedItem)
	{
		Config->AnsiSubtitleEncoding = static_cast<CharacterEncoding^>(cbEncodings->SelectedItem);
	}
}
