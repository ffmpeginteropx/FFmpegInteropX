#include "pch.h"
#include "FFmpegMediaSource.h"
#include "LanguageTagConverter.h"
#include "FFmpegMediaSource.g.cpp"
#include "winrt/Windows.ApplicationModel.Core.h"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropXWinUI::implementation
{
    // Static functions passed to FFmpeg
    static int FileStreamRead(void* ptr, uint8_t* buf, int bufSize);
    static int64_t FileStreamSeek(void* ptr, int64_t pos, int whence);

    // Flag for ffmpeg global setup
    static bool isRegistered = false;
    std::mutex isRegisteredMutex;

    FFmpegMediaSource::FFmpegMediaSource(MediaSourceConfig const& interopConfig, 
        CoreDispatcher const& dispatcher)
        : config(interopConfig)
        , thumbnailStreamIndex(AVERROR_STREAM_NOT_FOUND)
        , isFirstSeek(true)
        , dispatcher(dispatcher)
    {
        if (!isRegistered)
        {
            isRegisteredMutex.lock();
            if (!isRegistered)
            {
                LanguageTagConverter::Initialize();
                isRegistered = true;
            }

            isRegisteredMutex.unlock();
        }
        subtitleDelay = config.DefaultSubtitleDelay();
        audioStrInfos = winrt::single_threaded_vector<winrt::FFmpegInteropXWinUI::AudioStreamInfo>();
        subtitleStrInfos = winrt::single_threaded_vector<winrt::FFmpegInteropXWinUI::SubtitleStreamInfo>();
        videoStrInfos = winrt::single_threaded_vector<winrt::FFmpegInteropXWinUI::VideoStreamInfo>();
        auto implConfig = config.as<winrt::FFmpegInteropXWinUI::implementation::MediaSourceConfig>();
        if (!implConfig->IsExternalSubtitleParser && !implConfig->IsFrameGrabber)
        {
            metadata = std::shared_ptr<MediaMetadata>(new MediaMetadata());
        }
    }

    FFmpegMediaSource::~FFmpegMediaSource()
    {
        Close();
    }

    winrt::com_ptr<FFmpegMediaSource> FFmpegMediaSource::CreateFromStream(IRandomAccessStream const& stream, MediaSourceConfig const& config, CoreDispatcher const& dispatcher)
    {
        auto interopMSS = winrt::make_self<FFmpegMediaSource>(config, dispatcher);
        auto hr = interopMSS->CreateMediaStreamSource(stream);
        if (!SUCCEEDED(hr))
        {
           throw_hresult(hr);
        }
        return interopMSS;
    }

    winrt::com_ptr<FFmpegMediaSource> FFmpegMediaSource::CreateFromUri(hstring const& uri, MediaSourceConfig const& config, CoreDispatcher const& dispatcher)
    {
        auto interopMSS = winrt::make_self<FFmpegMediaSource>(config, dispatcher);
        auto hr = interopMSS->CreateMediaStreamSource(uri);
        if (!SUCCEEDED(hr))
        {
            throw_hresult(hr);
        }
        return interopMSS;
    }

    winrt::com_ptr<FFmpegMediaSource> FFmpegMediaSource::CreateFromUri(hstring const& uri, MediaSourceConfig const& config)
    {
        auto dispatcher = GetCurrentDispatcher();
        auto interopMSS = winrt::make_self<FFmpegMediaSource>(config, dispatcher);
        auto hr = interopMSS->CreateMediaStreamSource(uri);
        if (!SUCCEEDED(hr))
        {
            throw_hresult(hr);
        }
        return interopMSS;
    }

    HRESULT FFmpegMediaSource::CreateMediaStreamSource(IRandomAccessStream const& stream)
    {
        return S_OK;
    }

    HRESULT FFmpegMediaSource::CreateMediaStreamSource(hstring const& uri)
    {
        return S_OK;
    }

    CoreDispatcher FFmpegMediaSource::GetCurrentDispatcher()
    {
        try {
            //try get the current view
            auto wnd = CoreWindow::GetForCurrentThread();
            if (wnd == nullptr)
            {
                wnd = winrt::Windows::ApplicationModel::Core::CoreApplication::MainView().CoreWindow();
            }
            if (wnd != nullptr)
                return wnd.Dispatcher();

            return nullptr;
        }
        catch (...)
        {
            return nullptr;
        }
    }

    Windows::Foundation::IAsyncOperation<FFmpegInteropXWinUI::FFmpegMediaSource> FFmpegMediaSource::CreateFromStreamAsync(Windows::Storage::Streams::IRandomAccessStream stream, FFmpegInteropXWinUI::MediaSourceConfig config)
    {
        throw hresult_not_implemented();
    }

    Windows::Foundation::IAsyncOperation<FFmpegInteropXWinUI::FFmpegMediaSource> FFmpegMediaSource::CreateFromStreamAsync(Windows::Storage::Streams::IRandomAccessStream stream)
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::IAsyncOperation<FFmpegInteropXWinUI::FFmpegMediaSource> FFmpegMediaSource::CreateFromUriAsync(hstring uri, FFmpegInteropXWinUI::MediaSourceConfig config)
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::IAsyncOperation<FFmpegInteropXWinUI::FFmpegMediaSource> FFmpegMediaSource::CreateFromUriAsync(hstring uri)
    {
        throw hresult_not_implemented();
    }
    void FFmpegMediaSource::SetSubtitleDelay(Windows::Foundation::TimeSpan const& delay)
    {
        throw hresult_not_implemented();
    }
    void FFmpegMediaSource::SetFFmpegAudioFilters(hstring const& audioFilters)
    {
        throw hresult_not_implemented();
    }
    void FFmpegMediaSource::SetFFmpegVideoFilters(hstring const& videoEffects)
    {
        throw hresult_not_implemented();
    }
    void FFmpegMediaSource::DisableAudioEffects()
    {
        throw hresult_not_implemented();
    }
    void FFmpegMediaSource::DisableVideoEffects()
    {
        throw hresult_not_implemented();
    }
  
    FFmpegInteropXWinUI::MediaThumbnailData FFmpegMediaSource::ExtractThumbnail()
    {
        throw hresult_not_implemented();
    }
   
    Windows::Media::Core::MediaStreamSource FFmpegMediaSource::GetMediaStreamSource()
    {
        throw hresult_not_implemented();
    }
    
    Windows::Media::Playback::MediaPlaybackItem FFmpegMediaSource::CreateMediaPlaybackItem()
    {
        throw hresult_not_implemented();
    }
    
    Windows::Media::Playback::MediaPlaybackItem FFmpegMediaSource::CreateMediaPlaybackItem(Windows::Foundation::TimeSpan const& startTime)
    {
        throw hresult_not_implemented();
    }
   
    Windows::Media::Playback::MediaPlaybackItem FFmpegMediaSource::CreateMediaPlaybackItem(Windows::Foundation::TimeSpan const& startTime, Windows::Foundation::TimeSpan const& durationLimit)
    {
        throw hresult_not_implemented();
    }
   
    Windows::Foundation::IAsyncOperation<Windows::Foundation::Collections::IVectorView<FFmpegInteropXWinUI::SubtitleStreamInfo>> FFmpegMediaSource::AddExternalSubtitleAsync(Windows::Storage::Streams::IRandomAccessStream stream, hstring streamName)
    {
        throw hresult_not_implemented();
    }
   
    Windows::Foundation::IAsyncOperation<Windows::Foundation::Collections::IVectorView<FFmpegInteropXWinUI::SubtitleStreamInfo>> FFmpegMediaSource::AddExternalSubtitleAsync(Windows::Storage::Streams::IRandomAccessStream stream)
    {
        throw hresult_not_implemented();
    }
   
    FFmpegInteropXWinUI::MediaSourceConfig FFmpegMediaSource::Configuration()
    {
        throw hresult_not_implemented();
    }
  
    Windows::Foundation::Collections::IVectorView<Windows::Foundation::Collections::IKeyValuePair<hstring, hstring>> FFmpegMediaSource::MetadataTags()
    {
        throw hresult_not_implemented();
    }
   
    Windows::Foundation::TimeSpan FFmpegMediaSource::Duration()
    {
        throw hresult_not_implemented();
    }
   
    FFmpegInteropXWinUI::VideoStreamInfo FFmpegMediaSource::CurrentVideoStream()
    {
        throw hresult_not_implemented();
    }
   
    FFmpegInteropXWinUI::AudioStreamInfo FFmpegMediaSource::CurrentAudioStream()
    {
        throw hresult_not_implemented();
    }
   
    Windows::Foundation::Collections::IVectorView<FFmpegInteropXWinUI::VideoStreamInfo> FFmpegMediaSource::VideoStreams()
    {
        throw hresult_not_implemented();
    }
    
    Windows::Foundation::Collections::IVectorView<FFmpegInteropXWinUI::AudioStreamInfo> FFmpegMediaSource::AudioStreams()
    {
        throw hresult_not_implemented();
    }
    
    Windows::Foundation::Collections::IVectorView<FFmpegInteropXWinUI::SubtitleStreamInfo> FFmpegMediaSource::SubtitleStreams()
    {
        throw hresult_not_implemented();
    }
    
    Windows::Foundation::Collections::IVectorView<FFmpegInteropXWinUI::ChapterInfo> FFmpegMediaSource::ChapterInfos()
    {
        throw hresult_not_implemented();
    }
    
    FFmpegInteropXWinUI::FormatInfo FFmpegMediaSource::FormatInfo()
    {
        throw hresult_not_implemented();
    }
    
    bool FFmpegMediaSource::HasThumbnail()
    {
        throw hresult_not_implemented();
    }
   
    Windows::Media::Playback::MediaPlaybackItem FFmpegMediaSource::PlaybackItem()
    {
        throw hresult_not_implemented();
    }
    
    Windows::Foundation::TimeSpan FFmpegMediaSource::SubtitleDelay()
    {
        throw hresult_not_implemented();
    }
   
    Windows::Foundation::TimeSpan FFmpegMediaSource::BufferTime()
    {
        throw hresult_not_implemented();
    }
    
    Windows::Media::Playback::MediaPlaybackSession FFmpegMediaSource::PlaybackSession()
    {
        throw hresult_not_implemented();
    }
    
    void FFmpegMediaSource::PlaybackSession(Windows::Media::Playback::MediaPlaybackSession const& value)
    {
        throw hresult_not_implemented();
    }

    void FFmpegMediaSource::Close()
    {
        mutexGuard.lock();
        if (mss)
        {
            mss.Starting(startingRequestedToken);
            mss.SampleRequested(sampleRequestedToken);
            mss.SwitchStreamsRequested(switchStreamRequestedToken);
            mss = nullptr;
        }

        if (playbackItem)
        {
            playbackItem.AudioTracksChanged(audioTracksChangedToken);
            playbackItem.TimedMetadataTracks().PresentationModeChanged(subtitlePresentationModeChangedToken);
            playbackItem = nullptr;
        }

        // Clear our data
        currentAudioStream.reset();
        currentVideoStream.reset();

        if (m_pReader != nullptr)
        {
            m_pReader.reset();;
        }

        subtitleStreams.clear();
        sampleProviders.clear();
        audioStreams.clear();
        videoStreams.clear();

        avformat_close_input(&avFormatCtx);
        av_free(avIOCtx);
        av_dict_free(&avDict);

        if (fileStreamData != nullptr)
        {
            fileStreamData->Release();
        }
        if (avHardwareContext)
        {
            av_buffer_unref(&avHardwareContext);
        }
        if (avHardwareContextDefault)
        {
            av_buffer_unref(&avHardwareContextDefault);
        }

        if (deviceHandle && deviceManager)
            deviceManager->CloseDeviceHandle(deviceHandle);

        SAFE_RELEASE(device);
        SAFE_RELEASE(deviceContext);
        SAFE_RELEASE(deviceManager);

        PlaybackSession(nullptr);

        mutexGuard.unlock();
    }
}
