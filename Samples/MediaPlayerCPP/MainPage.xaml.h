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
#include "FloatToDoubleConverter.h"
#include "TimeSpanToDoubleConverter.h"

namespace MediaPlayerCPP
{
    using namespace Concurrency;

    public ref class MainPage sealed
    {
    public:
        MainPage();

        property FFmpegInteropX::MediaSourceConfig^ Config;
        property FFmpegInteropX::VideoEffectConfiguration^ VideoEffectConfiguration;

        double GetBufferSizeMB();
        void SetBufferSizeMB(double value);

    private:
        void OpenLocalFile(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
        task<void> OpenLocalFile();
        task<void> OpenLocalFile(Windows::Storage::StorageFile^ file);
        task<void> TryOpenLastFile();
        task<void> TryOpenLastUri();
        void URIBoxKeyUp(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e);
        task<void> OpenUriStream(Platform::String^ uri);
        void MediaFailed(Platform::Object^ sender, Windows::UI::Xaml::ExceptionRoutedEventArgs^ e);
        void ExtractFrame(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
        task<void> ExtractFrame();
        void DisplayErrorMessage(Platform::String^ message);
        void Page_Loaded(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
        void OnButtonPressed(Windows::Media::SystemMediaTransportControls^ sender, Windows::Media::SystemMediaTransportControlsButtonPressedEventArgs^ args);
        void LoadSubtitleFile(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
        task<void> LoadSubtitleFile();
        void LoadSubtitleFileFFmpeg(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
        task<void> LoadSubtitleFileFFmpeg();
        void OnResolved(Windows::Media::Core::TimedTextSource^ sender, Windows::Media::Core::TimedTextSourceResolveResultEventArgs^ args);
        void OnTimedMetadataTracksChanged(Windows::Media::Playback::MediaPlaybackItem^ sender, Windows::Foundation::Collections::IVectorChangedEventArgs^ args);
        void PassthroughVideo_Toggled(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

        Windows::Media::Playback::MediaPlayer^ mediaPlayer;
        Windows::Storage::StorageFile^ currentFile;
        FFmpegInteropX::FFmpegMediaSource^ actualFFmpegMSS;
        FFmpegInteropX::FFmpegMediaSource^ FFmpegMSS;
        Windows::Media::Playback::MediaPlaybackItem^ playbackItem;
        Windows::Foundation::EventRegistrationToken timedMetadataTracksChangedToken;
        void CbEncodings_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
        void AddTestFilter(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
        void RemoveTestFilter(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
        void DelaySubtitles(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
        void QuickenSubtitles(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
        void EnableVideoEffects_Toggled(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
        void AutoDetect_Toggled(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

        void OnKeyDown(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::KeyEventArgs^ args);
        void Page_DragEnter(Platform::Object^ sender, Windows::UI::Xaml::DragEventArgs^ e);
        void Page_Drop(Platform::Object^ sender, Windows::UI::Xaml::DragEventArgs^ e);
        void ffmpegVideoFilters_KeyDown(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e);
        void ffmpegAudioFilters_KeyDown(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e);
        void OnMediaOpened(Windows::Media::Playback::MediaPlayer^ sender, Platform::Object^ args);
        void OnMediaFailed(Windows::Media::Playback::MediaPlayer^ sender, Windows::Media::Playback::MediaPlayerFailedEventArgs^ args);

        void StreamDelayManipulation(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e);
    };
}
