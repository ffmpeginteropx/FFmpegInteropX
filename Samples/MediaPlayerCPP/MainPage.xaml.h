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
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"

namespace MediaPlayerCPP
{
	public ref class MainPage sealed
	{
	public:
		MainPage();

		property FFmpegInterop::FFmpegInteropConfig^ Config;

	private:
		void OpenLocalFile(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void URIBoxKeyUp(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e);
		void MediaFailed(Platform::Object^ sender, Windows::UI::Xaml::ExceptionRoutedEventArgs^ e);
		void ExtractFrame(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void DisplayErrorMessage(Platform::String^ message);
		void Page_Loaded(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void OnButtonPressed(Windows::Media::SystemMediaTransportControls ^sender, Windows::Media::SystemMediaTransportControlsButtonPressedEventArgs ^args);
		void LoadSubtitleFile(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void LoadSubtitleFileFFmpeg(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void OnResolved(Windows::Media::Core::TimedTextSource ^sender, Windows::Media::Core::TimedTextSourceResolveResultEventArgs ^args);
		void OnTimedMetadataTracksChanged(Windows::Media::Playback::MediaPlaybackItem ^sender, Windows::Foundation::Collections::IVectorChangedEventArgs ^args);
		void PassthroughVideo_Toggled(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

		Windows::Storage::StorageFile^ currentFile;
		FFmpegInterop::FFmpegInteropMSS^ FFmpegMSS;
		Windows::Media::Playback::MediaPlaybackItem^ playbackItem;
		Windows::Foundation::EventRegistrationToken timedMetadataTracksChangedToken;
		void CbEncodings_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
		void AutoDetect_Toggled(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	};
}
