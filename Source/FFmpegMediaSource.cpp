#include "pch.h"
#include "FFmpegMediaSource.h"
#include "LanguageTagConverter.h"
#include "FFmpegMediaSource.g.cpp"
#include "winrt/Windows.ApplicationModel.Core.h"
#include "D3D11VideoSampleProvider.h"
#include "H264AVCSampleProvider.h"
#include "UncompressedAudioSampleProvider.h"
#include "UncompressedFrameProvider.h"
#include "UncompressedSampleProvider.h"
#include "UncompressedVideoSampleProvider.h"
#include "HEVCSampleProvider.h"
#include "NALPacketSampleProvider.h"
#include "CodecChecker.h"
#include "MediaSampleProvider.h"
#include "SubtitleProviderSsaAss.h"
#include "SubtitleProviderBitmap.h"
#include "ChapterInfo.h"
#include "FFmpegReader.h"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
//static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::FFmpegInteropX::implementation
{
    using namespace Windows::Foundation;
    using namespace Windows::Storage::Streams;
    using namespace Windows::Media::Playback;

    // Static functions passed to FFmpeg
    static int FileStreamRead(void* ptr, uint8_t* buf, int bufSize);
    static int64_t FileStreamSeek(void* ptr, int64_t pos, int whence);
    static int IsShuttingDown(void* ptr);

    // Flag for ffmpeg global setup
    static bool isRegistered = false;
    std::mutex isRegisteredMutex;

    FFmpegMediaSource::FFmpegMediaSource(winrt::com_ptr<MediaSourceConfig> const& interopConfig,
        DispatcherQueue const& dispatcher)
        : config(interopConfig)
        , thumbnailStreamIndex(AVERROR_STREAM_NOT_FOUND)
        , isFirstSeek(true)
        , dispatcher(dispatcher)
    {
        avDict = NULL;
        avHardwareContext = NULL;
        avHardwareContextDefault = NULL;
        device = NULL;
        deviceContext = NULL;
        deviceHandle = NULL;
        deviceManager = NULL;

        if (!isRegistered)
        {
            std::lock_guard lock(isRegisteredMutex);
            if (!isRegistered)
            {
                LanguageTagConverter::Initialize();
                isRegistered = true;
            }
        }
        subtitleDelay = config->DefaultSubtitleDelay();
        audioStrInfos = winrt::single_threaded_observable_vector<winrt::FFmpegInteropX::AudioStreamInfo>();
        subtitleStrInfos = winrt::single_threaded_observable_vector<winrt::FFmpegInteropX::SubtitleStreamInfo>();
        videoStrInfos = winrt::single_threaded_observable_vector<winrt::FFmpegInteropX::VideoStreamInfo>();
        if (!config->IsExternalSubtitleParser && !config->IsFrameGrabber)
        {
            metadata = std::shared_ptr<MediaMetadata>(new MediaMetadata());
        }
    }

    FFmpegMediaSource::~FFmpegMediaSource()
    {
        Close();
    }

    winrt::com_ptr<FFmpegMediaSource> FFmpegMediaSource::CreateFromStream(IRandomAccessStream const& stream, winrt::com_ptr<MediaSourceConfig> const& config, DispatcherQueue const& dispatcher)
    {
        auto interopMSS = winrt::make_self<FFmpegMediaSource>(config, dispatcher);
        auto hr = interopMSS->CreateMediaStreamSource(stream);
        if (!SUCCEEDED(hr))
        {
            throw_hresult(hr);
        }
        return interopMSS;
    }

    winrt::com_ptr<FFmpegMediaSource> FFmpegMediaSource::CreateFromUri(hstring const& uri, winrt::com_ptr<MediaSourceConfig> const& config, DispatcherQueue const& dispatcher)
    {
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
        HRESULT hr = S_OK;
        if (!stream)
        {
            hr = E_INVALIDARG;
        }

        if (SUCCEEDED(hr))
        {
            // Convert asynchronous IRandomAccessStream to synchronous IStream. This API requires shcore.h and shcore.lib
            //
            //
            hr = CreateStreamOverRandomAccessStream(reinterpret_cast<::IUnknown*>(winrt::get_abi(stream)), IID_PPV_ARGS(&fileStreamData));

        }

        unsigned char* fileStreamBuffer = NULL;
        if (SUCCEEDED(hr))
        {
            // Setup FFmpeg custom IO to access file as stream. This is necessary when accessing any file outside of app installation directory and appdata folder.
            // Credit to Philipp Sch http://www.codeproject.com/Tips/489450/Creating-Custom-FFmpeg-IO-Context
            fileStreamBuffer = (unsigned char*)av_malloc(config->FileStreamReadSize());
            if (fileStreamBuffer == nullptr)
            {
                hr = E_OUTOFMEMORY;
            }
        }

        if (SUCCEEDED(hr))
        {
            avIOCtx = avio_alloc_context(fileStreamBuffer, config->StreamBufferSize(), 0, (void*)winrt::get_abi(this), FileStreamRead, 0, FileStreamSeek);
            if (avIOCtx == nullptr)
            {
                av_free(fileStreamBuffer);
                hr = E_OUTOFMEMORY;
            }
        }

        if (SUCCEEDED(hr))
        {
            avFormatCtx = avformat_alloc_context();
            if (avFormatCtx == nullptr)
            {
                hr = E_OUTOFMEMORY;
            }
        }

        if (SUCCEEDED(hr))
        {
            // Populate AVDictionary avDict based on PropertySet ffmpegOptions. List of options can be found in https://www.ffmpeg.org/ffmpeg-protocols.html
            hr = ParseOptions(config->FFmpegOptions());
        }


        if (SUCCEEDED(hr))
        {
            // Populate AVDictionary avDict based on additional ffmpegOptions. List of options can be found in https://www.ffmpeg.org/ffmpeg-protocols.html
            hr = ParseOptions(config->AdditionalFFmpegSubtitleOptions);
        }

        if (SUCCEEDED(hr))
        {
            // Register callback for fast dispose
            avFormatCtx->interrupt_callback.callback = IsShuttingDown;
            avFormatCtx->interrupt_callback.opaque = this;
        }

        if (SUCCEEDED(hr))
        {
            avFormatCtx->pb = avIOCtx;
            avFormatCtx->flags |= AVFMT_FLAG_CUSTOM_IO;
            // Open media file using custom IO setup above instead of using file name. Opening a file using file name will invoke fopen C API call that only have
            // access within the app installation directory and appdata folder. Custom IO allows access to file selected using FilePicker dialog.
            if (avformat_open_input(&avFormatCtx, "", NULL, &avDict) < 0)
            {
                hr = E_FAIL; // Error opening file
            }

            // avDict is not NULL only when there is an issue with the given ffmpegOptions such as invalid key, value type etc. Iterate through it to see which one is causing the issue.
            if (avDict != nullptr)
            {
                DebugMessage(L"Invalid FFmpeg option(s)");
                av_dict_free(&avDict);

                avDict = nullptr;
            }
        }

        if (SUCCEEDED(hr))
        {
            this->mss = mss;
            hr = InitFFmpegContext();
        }

        return hr;
    }

    HRESULT FFmpegMediaSource::CreateMediaStreamSource(hstring const& uri)
    {
        HRESULT hr = S_OK;
        if (uri.empty())
        {
            hr = E_INVALIDARG;
        }

        if (SUCCEEDED(hr))
        {
            avFormatCtx = avformat_alloc_context();
            if (avFormatCtx == nullptr)
            {
                hr = E_OUTOFMEMORY;
            }
        }

        if (SUCCEEDED(hr))
        {
            // Populate AVDictionary avDict based on PropertySet ffmpegOptions. List of options can be found in https://www.ffmpeg.org/ffmpeg-protocols.html
            hr = ParseOptions(config->FFmpegOptions());
        }

        if (SUCCEEDED(hr))
        {
            // Register callback for fast dispose
            avFormatCtx->interrupt_callback.callback = IsShuttingDown;
            avFormatCtx->interrupt_callback.opaque = this;
        }

        if (SUCCEEDED(hr))
        {
            auto charStr = StringUtils::PlatformStringToUtf8String(uri);

            // Open media in the given URI using the specified options
            if (avformat_open_input(&avFormatCtx, charStr.c_str(), NULL, &avDict) < 0)
            {
                hr = E_FAIL; // Error opening file
            }

            // avDict is not NULL only when there is an issue with the given ffmpegOptions such as invalid key, value type etc. Iterate through it to see which one is causing the issue.
            if (avDict != nullptr)
            {
                DebugMessage(L"Invalid FFmpeg option(s)");
                av_dict_free(&avDict);
                avDict = nullptr;
            }
        }

        if (SUCCEEDED(hr))
        {
            this->mss = nullptr;
            hr = InitFFmpegContext();
        }

        return hr;
    }

    void FFmpegMediaSource::OnPresentationModeChanged(MediaPlaybackTimedMetadataTrackList const& sender, TimedMetadataPresentationModeChangedEventArgs const& args)
    {
        UNREFERENCED_PARAMETER(sender);
        std::lock_guard lock(mutex);
        if (mss == nullptr)
        {
            return;
        }

        int index = 0;
        for (auto& stream : subtitleStreams)
        {
            if (stream->SubtitleTrack == args.Track())
            {
                auto mode = args.NewPresentationMode();
                if (mode == TimedMetadataTrackPresentationMode::Disabled || mode == TimedMetadataTrackPresentationMode::Hidden)
                {
                    stream->DisableStream();
                }
                else
                {
                    stream->EnableStream();
                }
            }
            index++;
        }
    }

    void FFmpegMediaSource::OnAudioTracksChanged(MediaPlaybackItem const& sender, IVectorChangedEventArgs const& args)
    {
        UNREFERENCED_PARAMETER(args);
        std::lock_guard lock(mutex);
        if (mss == nullptr)
        {
            return;
        }
        if (sender.AudioTracks().Size() == AudioStreams().Size())
        {
            for (unsigned int i = 0; i < AudioStreams().Size(); i++)
            {
                auto track = sender.AudioTracks().GetAt(i);
                auto info = AudioStreams().GetAt(i);
                if (!info.Name().empty())
                {
                    track.Label(info.Name());
                }
                else if (!info.Language().empty())
                {
                    track.Label(info.Language());
                }
            }
        }
    }

    void FFmpegMediaSource::InitializePlaybackItem(MediaPlaybackItem const& playbackitem)
    {
        audioTracksChangedToken = playbackitem.AudioTracksChanged({ get_weak(), &FFmpegInteropX::implementation::FFmpegMediaSource::OnAudioTracksChanged });
        subtitlePresentationModeChangedToken = playbackitem.TimedMetadataTracks().PresentationModeChanged({ get_weak(), &FFmpegInteropX::implementation::FFmpegMediaSource::OnPresentationModeChanged });

        if (config->AutoSelectForcedSubtitles())
        {
            int index = 0;
            for (auto stream : subtitleStreamInfos)
            {
                if (stream.IsForced())
                {
                    playbackitem.TimedMetadataTracks().SetPresentationMode(index, TimedMetadataTrackPresentationMode::PlatformPresented);
                    break;
                }

                index++;
            }
        }

        for (auto& stream : subtitleStreams)
        {
            stream->PlaybackItem = playbackItem;
        }
    }


    DispatcherQueue FFmpegMediaSource::GetCurrentDispatcher()
    {
        try
        {
            DispatcherQueue dispatcherQueue = DispatcherQueue::GetForCurrentThread();
            //try get the current view      
            return dispatcherQueue;
        }
        catch (...)
        {
            return nullptr;
        }
    }

    IAsyncOperation<FFmpegInteropX::FFmpegMediaSource> FFmpegMediaSource::CreateFromStreamAsync(IRandomAccessStream stream, FFmpegInteropX::MediaSourceConfig config)
    {
        winrt::apartment_context caller; // Capture calling context.
        auto dispatcher = GetCurrentDispatcher();
        auto configImpl = config.as<winrt::FFmpegInteropX::implementation::MediaSourceConfig>();
        CheckUseHdr(configImpl);
        co_await winrt::resume_background();
        auto result = CreateFromStream(stream, configImpl, dispatcher);
        co_await caller;
        co_return result.as<FFmpegInteropX::FFmpegMediaSource>();;
    }

    IAsyncOperation<FFmpegInteropX::FFmpegMediaSource> FFmpegMediaSource::CreateFromStreamAsync(IRandomAccessStream stream)
    {
        return CreateFromStreamAsync(stream, FFmpegInteropX::MediaSourceConfig());
    }

    IAsyncOperation<FFmpegInteropX::FFmpegMediaSource> FFmpegMediaSource::CreateFromUriAsync(hstring uri, FFmpegInteropX::MediaSourceConfig config)
    {
        winrt::apartment_context caller; // Capture calling context.
        auto dispatcher = GetCurrentDispatcher();
        auto configImpl = config.as<winrt::FFmpegInteropX::implementation::MediaSourceConfig>();
        CheckUseHdr(configImpl);
        co_await winrt::resume_background();
        auto result = CreateFromUri(uri, configImpl, dispatcher);
        co_await caller;
        co_return result.as<FFmpegInteropX::FFmpegMediaSource>();
    }

    IAsyncOperation<FFmpegInteropX::FFmpegMediaSource> FFmpegMediaSource::CreateFromUriAsync(hstring uri)
    {
        return CreateFromUriAsync(uri, FFmpegInteropX::MediaSourceConfig());
    }

    static int is_hwaccel_pix_fmt(enum AVPixelFormat pix_fmt)
    {
        const AVPixFmtDescriptor* desc = av_pix_fmt_desc_get(pix_fmt);
        return desc->flags & AV_PIX_FMT_FLAG_HWACCEL;
    }

    static AVPixelFormat get_format(struct AVCodecContext* s, const enum AVPixelFormat* fmt)
    {
        AVPixelFormat result_sw = (AVPixelFormat)-1;
        AVPixelFormat result_hw = (AVPixelFormat)-1;
        AVPixelFormat format;
        int index = 0;
        do
        {
            format = fmt[index++];

            //		
            if (format != -1)
            {
                if (s->hw_device_ctx && format == AV_PIX_FMT_D3D11)
                {
                    // we only support D3D11 HW format (not D3D11_VLD)
                    result_hw = format;
                }
                else if (result_sw == -1 && !is_hwaccel_pix_fmt(format))
                {
                    // take first non hw accelerated format
                    result_sw = format;
                }
                else if (format == AV_PIX_FMT_NV12 && result_sw != AV_PIX_FMT_YUVA420P)
                {
                    // switch SW format to NV12 if available, unless this is an alpha channel file
                    result_sw = format;
                }
            }
        } while (format != -1);


        if (result_hw != -1)
        {
            return result_hw;
        }
        else
        {
            return result_sw;
        }
    }

    HRESULT FFmpegMediaSource::InitFFmpegContext()
    {
        HRESULT hr = S_OK;

        if (SUCCEEDED(hr))
        {
            if (avformat_find_stream_info(avFormatCtx, NULL) < 0)
            {
                hr = E_FAIL; // Error finding info
            }
        }

        if (SUCCEEDED(hr))
        {
            m_pReader = std::shared_ptr<FFmpegReader>(new FFmpegReader(avFormatCtx, &sampleProviders, config.as<FFmpegInteropX::MediaSourceConfig>()));
            if (m_pReader == nullptr)
            {
                hr = E_OUTOFMEMORY;
            }
        }

        // do not use start time for pure subtitle files
        if (config->IsExternalSubtitleParser && avFormatCtx->nb_streams == 1)
        {
            avFormatCtx->start_time = AV_NOPTS_VALUE;
        }

        const AVCodec* avVideoCodec;
        auto videoStreamIndex = av_find_best_stream(avFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &avVideoCodec, 0);
        auto audioStreamIndex = av_find_best_stream(avFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
        auto subtitleStreamIndex = av_find_best_stream(avFormatCtx, AVMEDIA_TYPE_SUBTITLE, -1, -1, NULL, 0);

        attachedFileHelper = shared_ptr<AttachedFileHelper>(new AttachedFileHelper(config.as<winrt::FFmpegInteropX::MediaSourceConfig>()));

        // first parse attached files, so they are available for subtitle streams during initialize
        if (config->UseEmbeddedSubtitleFonts())
        {
            for (unsigned int index = 0; index < avFormatCtx->nb_streams; index++)
            {
                auto avStream = avFormatCtx->streams[index];
                if (avStream->codecpar->codec_type == AVMEDIA_TYPE_ATTACHMENT)
                {
                    auto fileName = av_dict_get(avStream->metadata, "filename", NULL, 0);
                    auto mimetype = av_dict_get(avStream->metadata, "mimetype", NULL, 0);
                    if (fileName && avStream->codecpar->extradata && avStream->codecpar->extradata_size > 0)
                    {
                        auto name = StringUtils::Utf8ToPlatformString(fileName->value);
                        auto mime = mimetype ? StringUtils::Utf8ToPlatformString(mimetype->value) : L"";

                        auto file = std::shared_ptr<AttachedFile>(new AttachedFile(name, mime, avStream));
                        attachedFileHelper->AddAttachedFile(file);
                    }
                }
            }
        }

        for (int index = 0; index < (int)avFormatCtx->nb_streams; index++)
        {
            auto avStream = avFormatCtx->streams[index];
            avStream->discard = AVDISCARD_ALL; // all streams are disabled until we enable them

            std::shared_ptr<MediaSampleProvider> stream = nullptr;

            if (avStream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO && !config->IsFrameGrabber && !config->IsExternalSubtitleParser)
            {
                stream = CreateAudioStream(avStream, index);
                if (stream)
                {
                    if (index == audioStreamIndex)
                    {
                        stream->AudioInfo().as<implementation::AudioStreamInfo>()->SetDefault();
                        currentAudioStream = stream;
                        audioStrInfos.InsertAt(0, stream->AudioInfo());
                        audioStreams.insert(audioStreams.begin(), stream);
                    }
                    else
                    {
                        audioStrInfos.Append(stream->AudioInfo());
                        audioStreams.push_back(stream);
                    }
                }
            }
            else if (avStream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO && avStream->disposition == AV_DISPOSITION_ATTACHED_PIC && thumbnailStreamIndex == AVERROR_STREAM_NOT_FOUND && !config->IsExternalSubtitleParser)
            {
                thumbnailStreamIndex = index;
            }
            else if (avStream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO && !config->IsExternalSubtitleParser)
            {
                stream = CreateVideoStream(avStream, index);
                if (stream)
                {
                    if (index == videoStreamIndex)
                    {
                        stream->VideoInfo().as<implementation::VideoStreamInfo>()->isDefault = true;
                        currentVideoStream = stream;
                        videoStreams.insert(videoStreams.begin(), stream);
                        videoStrInfos.InsertAt(0, stream->VideoInfo());
                    }
                    else
                    {
                        videoStreams.push_back(stream);
                        videoStrInfos.Append(stream->VideoInfo());
                    }
                }
            }
            else if (avStream->codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE)
            {
                stream = CreateSubtitleSampleProvider(avStream, index);
                if (stream)
                {
                    if (index == subtitleStreamIndex)
                    {
                        stream->SubtitleInfo().as<implementation::SubtitleStreamInfo>()->SetDefault();
                        subtitleStrInfos.InsertAt(0, stream->SubtitleInfo());
                        subtitleStreams.insert(subtitleStreams.begin(), (std::reinterpret_pointer_cast<SubtitleProvider>(stream)));
                    }
                    else
                    {
                        subtitleStrInfos.Append(stream->SubtitleInfo());
                        subtitleStreams.push_back((std::reinterpret_pointer_cast<SubtitleProvider>(stream)));
                    }

                    // enable all subtitle streams for external subtitle parsing
                    if (config->IsExternalSubtitleParser)
                    {
                        (std::reinterpret_pointer_cast<SubtitleProvider>(stream))->EnableStream();
                    }
                }
            }

            sampleProviders.push_back(stream);
        }

        if (!currentAudioStream && audioStreams.size() > 0)
        {
            currentAudioStream = audioStreams[0];
        }
        if (!currentVideoStream && videoStreams.size() > 0)
        {
            currentVideoStream = videoStreams[0];
        }

        audioStreamInfos = audioStrInfos.GetView();
        subtitleStreamInfos = subtitleStrInfos.GetView();
        videoStreamInfos = videoStrInfos.GetView();



        if (currentVideoStream)
        {
            auto videoDescriptor = (currentVideoStream->StreamDescriptor().as<VideoStreamDescriptor>());
            auto encodingProperties = videoDescriptor.EncodingProperties();
            auto pixelAspect = (double)encodingProperties.PixelAspectRatio().Numerator() / encodingProperties.PixelAspectRatio().Denominator();
            auto videoAspect = ((double)encodingProperties.Width() / encodingProperties.Height()) / pixelAspect;
            for (auto& stream : subtitleStreams)
            {
                stream->NotifyVideoFrameSize(encodingProperties.Width(), encodingProperties.Height(), videoAspect);
            }
        }

        if (currentVideoStream && currentAudioStream)
        {
            mss = MediaStreamSource(currentVideoStream->StreamDescriptor(), currentAudioStream->StreamDescriptor());
            currentVideoStream->EnableStream();
            currentAudioStream->EnableStream();
        }
        else if (currentAudioStream)
        {
            mss = MediaStreamSource(currentAudioStream->StreamDescriptor());
            currentAudioStream->EnableStream();
        }
        else if (currentVideoStream)
        {
            mss = MediaStreamSource(currentVideoStream->StreamDescriptor());
            currentVideoStream->EnableStream();
        }
        else if (subtitleStreams.size() == 0 || !config->IsExternalSubtitleParser)
        {
            //only fail if there are no media streams (audio, video, or subtitle)
            hr = E_FAIL;
        }
        //if the streams are subtitles only, there will be no media stream source
        if (mss != nullptr) {
            if (SUCCEEDED(hr))
            {
                for (auto& stream : audioStreams)
                {
                    if (stream != currentAudioStream)
                    {
                        mss.AddStreamDescriptor(stream->StreamDescriptor());
                    }
                }

                for (auto& stream : videoStreams)
                {
                    if (stream != currentVideoStream)
                    {
                        mss.AddStreamDescriptor(stream->StreamDescriptor());
                    }
                }

                auto chapters = winrt::single_threaded_observable_vector<winrt::FFmpegInteropX::ChapterInfo>();
                if (avFormatCtx->chapters && avFormatCtx->nb_chapters > 1)
                {
                    for (size_t i = 0; i < avFormatCtx->nb_chapters; i++)
                    {
                        auto chapter = avFormatCtx->chapters[i];
                        auto entry = av_dict_get(chapter->metadata, "title", NULL, AV_DICT_IGNORE_SUFFIX);
                        if (entry)
                        {
                            auto title = StringUtils::Utf8ToPlatformString(entry->value);
                            TimeSpan start{ (long long)((chapter->start / (double)chapter->time_base.den) * chapter->time_base.num * 10000000) };
                            TimeSpan duration{ (long long)(((chapter->end - chapter->start) / (double)chapter->time_base.den) * chapter->time_base.num * 10000000) };

                            // compensate for start time offset
                            if (avFormatCtx->start_time != AV_NOPTS_VALUE)
                            {
                                start = TimeSpan(start.count() - (avFormatCtx->start_time * 10));
                            }

                            // cut off negative start times
                            if (start.count() < 0)
                            {
                                duration = TimeSpan(duration.count() + start.count());
                                start = TimeSpan(0);
                            }

                            if (duration.count() > 0)
                            {
                                auto chapInfo = winrt::FFmpegInteropX::ChapterInfo(title, start, duration);
                                chapters.Append(chapInfo);
                            }
                        }
                    }
                }
                chapterInfos = chapters.GetView();
            }

            if (SUCCEEDED(hr))
            {
                // Convert media duration from AV_TIME_BASE to TimeSpan unit
                mediaDuration = TimeSpan(LONGLONG(avFormatCtx->duration * 10000000 / double(AV_TIME_BASE)));

                // Assign initial BufferTime to MediaStreamSource
                mss.BufferTime(TimeSpan{ 0 });
                mss.MaxSupportedPlaybackRate(config->MaxSupportedPlaybackRate());

                if (mediaDuration.count() > 0)
                {
                    mss.Duration(mediaDuration);
                    mss.CanSeek(true);
                }

                auto title = av_dict_get(avFormatCtx->metadata, "title", NULL, 0);
                auto titleStr = title ? StringUtils::Utf8ToPlatformString(title->value) : L"";
                auto codecStr = StringUtils::Utf8ToPlatformString(avFormatCtx->iformat->name);
                formatInfo = winrt::FFmpegInteropX::FormatInfo(titleStr, codecStr, mediaDuration, avFormatCtx->bit_rate);

                // using strong reference here would create circle references, since we store MSS and MediaPlaybackItem here.
                startingRequestedToken = mss.Starting({ get_weak(), &FFmpegMediaSource::OnStarting });
                sampleRequestedToken = mss.SampleRequested({ get_weak(), &FFmpegMediaSource::OnSampleRequested });
                switchStreamRequestedToken = mss.SwitchStreamsRequested({ get_weak(), &FFmpegMediaSource::OnSwitchStreamsRequested });
            }

            if (!config->FFmpegVideoFilters().empty())
            {
                SetFFmpegVideoFilters(config->FFmpegVideoFilters());
            }

            if (!config->FFmpegAudioFilters().empty())
            {
                SetFFmpegAudioFilters(config->FFmpegAudioFilters());
            }
        }

        return hr;
    }


    std::shared_ptr<SubtitleProvider> FFmpegMediaSource::CreateSubtitleSampleProvider(AVStream* avStream, int index)
    {
        HRESULT hr = S_OK;
        std::shared_ptr<SubtitleProvider> avSubsStream = nullptr;
        auto avSubsCodec = avcodec_find_decoder(avStream->codecpar->codec_id);
        if (avSubsCodec)
        {
            // allocate a new decoding context
            auto avSubsCodecCtx = avcodec_alloc_context3(avSubsCodec);
            if (!avSubsCodecCtx)
            {
                DebugMessage(L"Could not allocate a decoding context\n");
                hr = E_OUTOFMEMORY;
            }

            //inject custom properties
            if (config->AutoCorrectAnsiSubtitles() && config->IsExternalSubtitleParser && streamByteOrderMark != ByteOrderMark::UTF8)
            {
                hstring key = config->AnsiSubtitleEncoding().Name();
                std::string keyA = StringUtils::PlatformStringToUtf8String(key);
                const char* keyChar = keyA.c_str();

                if (av_opt_set(avSubsCodecCtx, "sub_charenc", keyChar, AV_OPT_SEARCH_CHILDREN) < 0)
                {
                    DebugMessage(L"Could not set sub_charenc on subtitle provider\n");
                }
                if (av_opt_set_int(avSubsCodecCtx, "sub_charenc_mode", FF_SUB_CHARENC_MODE_AUTOMATIC, AV_OPT_SEARCH_CHILDREN) < 0)
                {
                    DebugMessage(L"Could not set sub_charenc_mode on subtitle provider\n");
                }
            }

            if (SUCCEEDED(hr))
            {
                // initialize the stream parameters with demuxer information
                if (avcodec_parameters_to_context(avSubsCodecCtx, avStream->codecpar) < 0)
                {
                    hr = E_FAIL;
                }

                if (SUCCEEDED(hr))
                {
                    if (avcodec_open2(avSubsCodecCtx, avSubsCodec, NULL) < 0)
                    {
                        hr = E_FAIL;
                    }
                    else
                    {
                        if ((avSubsCodecCtx->codec_descriptor->props & AV_CODEC_PROP_TEXT_SUB) == AV_CODEC_PROP_TEXT_SUB)
                        {
                            avSubsStream = std::shared_ptr<SubtitleProvider>(new SubtitleProviderSsaAss(m_pReader, avFormatCtx, avSubsCodecCtx, config.as<winrt::FFmpegInteropX::MediaSourceConfig>(), index, dispatcher, attachedFileHelper));
                        }
                        else if ((avSubsCodecCtx->codec_descriptor->props & AV_CODEC_PROP_BITMAP_SUB) == AV_CODEC_PROP_BITMAP_SUB)
                        {
                            avSubsStream = std::shared_ptr<SubtitleProvider>(new SubtitleProviderBitmap(m_pReader, avFormatCtx, avSubsCodecCtx, config.as<winrt::FFmpegInteropX::MediaSourceConfig>(), index, dispatcher));
                        }
                        else
                        {
                            hr = E_FAIL;
                        }
                    }
                }
            }

            if (SUCCEEDED(hr))
            {
                avSubsStream->SubtitleDelay = SubtitleDelay();
                hr = avSubsStream->Initialize();
            }

            if (FAILED(hr))
            {
                avSubsStream = nullptr;
            }

            // free codec context if failed
            if (!avSubsStream && avSubsCodecCtx)
            {
                avcodec_free_context(&avSubsCodecCtx);
            }
        }
        else
        {
            DebugMessage(L"Could not find decoder\n");
        }

        return avSubsStream;
    }


    std::shared_ptr<MediaSampleProvider> FFmpegMediaSource::CreateAudioStream(AVStream* avStream, int index)
    {
        HRESULT hr = S_OK;
        std::shared_ptr<MediaSampleProvider> audioStream = nullptr;
        auto avAudioCodec = avcodec_find_decoder(avStream->codecpar->codec_id);
        if (avAudioCodec)
        {
            // allocate a new decoding context
            auto avAudioCodecCtx = avcodec_alloc_context3(avAudioCodec);
            if (!avAudioCodecCtx)
            {
                DebugMessage(L"Could not allocate a decoding context\n");
                hr = E_OUTOFMEMORY;
            }

            if (SUCCEEDED(hr))
            {
                // initialize the stream parameters with demuxer information
                if (avcodec_parameters_to_context(avAudioCodecCtx, avStream->codecpar) < 0)
                {
                    hr = E_FAIL;
                }

                if (SUCCEEDED(hr))
                {
                    if (avAudioCodecCtx->sample_fmt == AV_SAMPLE_FMT_S16P)
                    {
                        avAudioCodecCtx->request_sample_fmt = AV_SAMPLE_FMT_S16;
                    }
                    else if (avAudioCodecCtx->sample_fmt == AV_SAMPLE_FMT_S32P)
                    {
                        avAudioCodecCtx->request_sample_fmt = AV_SAMPLE_FMT_S32;
                    }
                    else if (avAudioCodecCtx->sample_fmt == AV_SAMPLE_FMT_FLTP)
                    {
                        avAudioCodecCtx->request_sample_fmt = AV_SAMPLE_FMT_FLT;
                    }

                    // enable multi threading
                    unsigned threads = std::thread::hardware_concurrency();
                    if (threads > 0)
                    {
                        avAudioCodecCtx->thread_count = config->MaxAudioThreads() == 0 ? threads : min((int)threads, config->MaxAudioThreads());
                        avAudioCodecCtx->thread_type = FF_THREAD_FRAME | FF_THREAD_SLICE;
                    }

                    if (avcodec_open2(avAudioCodecCtx, avAudioCodec, NULL) < 0)
                    {
                        hr = E_FAIL;
                    }
                    else if (avAudioCodecCtx->sample_fmt == AV_SAMPLE_FMT_NONE)
                    {
                        hr = E_FAIL;
                    }
                    else
                    {
                        // Detect audio format and create audio stream descriptor accordingly
                        audioStream = CreateAudioSampleProvider(avStream, avAudioCodecCtx, index);
                    }
                }
            }

            // free codec context if failed
            if (!audioStream && avAudioCodecCtx)
            {
                avcodec_free_context(&avAudioCodecCtx);
            }
        }
        else
        {
            DebugMessage(L"Could not find decoder\n");
        }

        return audioStream;
    }


    std::shared_ptr<MediaSampleProvider> FFmpegMediaSource::CreateVideoStream(AVStream* avStream, int index)
    {
        HRESULT hr = S_OK;
        std::shared_ptr<MediaSampleProvider> result = nullptr;

        // Find the video stream and its decoder
        auto avVideoCodec = avcodec_find_decoder(avStream->codecpar->codec_id);

        if (avVideoCodec)
        {
            auto tryAv1hw = avVideoCodec->id == AVCodecID::AV_CODEC_ID_AV1 && std::string(avVideoCodec->name) != "av1" && config->VideoDecoderMode() == VideoDecoderMode::Automatic;
            auto libdav1d = tryAv1hw ? avVideoCodec : NULL;
            if (tryAv1hw)
            {
                avVideoCodec = avcodec_find_decoder_by_name("av1");
                if (!avVideoCodec)
                {
                    avVideoCodec = libdav1d;
                }
            }

            // allocate a new decoding context
            auto avVideoCodecCtx = avcodec_alloc_context3(avVideoCodec);
            if (!avVideoCodecCtx)
            {
                DebugMessage(L"Could not allocate a decoding context\n");
                hr = E_OUTOFMEMORY;
            }

            // create and assign HW device context, if supported and requested
            if (SUCCEEDED(hr) && config->VideoDecoderMode() == VideoDecoderMode::Automatic)
            {
                int i = 0;
                while (SUCCEEDED(hr))
                {
                    auto hwConfig = avcodec_get_hw_config(avVideoCodec, i++);
                    if (hwConfig)
                    {
                        if (hwConfig->pix_fmt == AV_PIX_FMT_D3D11 && hwConfig->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX)
                        {
                            AVBufferRef* hwContext = NULL;
                            if (!avHardwareContext)
                            {
                                avHardwareContext = av_hwdevice_ctx_alloc(AVHWDeviceType::AV_HWDEVICE_TYPE_D3D11VA);
                            }

                            if (!avHardwareContext)
                            {
                                hr = E_FAIL;
                            }

                            if (avVideoCodecCtx->codec_id == AV_CODEC_ID_VC1 || avVideoCodecCtx->codec_id == AV_CODEC_ID_WMV3)
                            {
                                // workaround for VC1 and WMV3: use default device context, later replace with actual MSS device context
                                if (!avHardwareContextDefault)
                                {
                                    if (SUCCEEDED(hr))
                                    {
                                        hr = av_hwdevice_ctx_create(&avHardwareContextDefault, AVHWDeviceType::AV_HWDEVICE_TYPE_D3D11VA, NULL, NULL, 0);
                                    }

                                    if (SUCCEEDED(hr))
                                    {
                                        hr = av_hwdevice_ctx_init(avHardwareContextDefault);
                                    }

                                    if (FAILED(hr) && avHardwareContextDefault)
                                    {
                                        av_buffer_unref(&avHardwareContextDefault);
                                    }
                                }

                                if (SUCCEEDED(hr))
                                {
                                    hwContext = avHardwareContextDefault;
                                }
                            }
                            else
                            {
                                if (SUCCEEDED(hr))
                                {
                                    hwContext = avHardwareContext;
                                }
                            }

                            if (hwContext)
                            {
                                avVideoCodecCtx->hw_device_ctx = av_buffer_ref(hwContext);
                            }
                            else
                            {
                                hr = E_OUTOFMEMORY;
                            }
                            break;
                        }
                    }
                    else
                    {
                        break;
                    }
                }
            }

            if (SUCCEEDED(hr))
            {
                avVideoCodecCtx->get_format = &get_format;

                // initialize the stream parameters with demuxer information
                hr = avcodec_parameters_to_context(avVideoCodecCtx, avStream->codecpar);
            }

            if (SUCCEEDED(hr))
            {
                // enable multi threading only for SW decoders
                if (!avVideoCodecCtx->hw_device_ctx)
                {
                    unsigned threads = std::thread::hardware_concurrency();
                    avVideoCodecCtx->thread_count = config->MaxVideoThreads() == 0 ? threads : min((int)threads, config->MaxVideoThreads());
                    avVideoCodecCtx->thread_type = config->IsFrameGrabber ? FF_THREAD_SLICE : FF_THREAD_FRAME | FF_THREAD_SLICE;
                }

                hr = avcodec_open2(avVideoCodecCtx, avVideoCodec, NULL);
            }

            if (SUCCEEDED(hr))
            {
                // Detect video format and create video stream descriptor accordingly
                result = CreateVideoSampleProvider(avStream, avVideoCodecCtx, index);
            }

            // free codec context if failed
            if (!result && avVideoCodecCtx)
            {
                avcodec_free_context(&avVideoCodecCtx);
            }
        }

        return result;
    }



    void FFmpegMediaSource::SetSubtitleDelay(TimeSpan const& delay)
    {
        std::lock_guard lock(mutex);
        if (mss == nullptr)
        {
            return;
        }
        try
        {
            for (auto& subtitleStream : subtitleStreams)
            {
                subtitleStream->SetSubtitleDelay(delay);
            }

            subtitleDelay = delay;
        }
        catch (...)
        {
        }
    }

    void FFmpegMediaSource::SetFFmpegAudioFilters(hstring const& audioFilters)
    {
        std::lock_guard lock(mutex);
        if (mss == nullptr)
        {
            return;
        }
        for(auto audioStream: audioStreams)
        {
            audioStream->SetFFmpegFilters(audioFilters);
        }
    }

    void FFmpegMediaSource::SetFFmpegAudioFilters(hstring const& audioFilters, winrt::FFmpegInteropX::AudioStreamInfo const& audioStream)
    {
        std::lock_guard lock(mutex);
        if (mss == nullptr)
        {
            return;
        }
        for (auto internalAudioStream : audioStreams)
        {
            if (internalAudioStream->AudioInfo() == audioStream)
            {
                internalAudioStream->SetFFmpegFilters(audioFilters);
                break;
            }
        }
    }

    void FFmpegMediaSource::SetFFmpegVideoFilters(hstring const& videoFilters)
    {
        std::lock_guard lock(mutex);
        if (mss == nullptr)
        {
            return;
        }

        for (auto videoStream : videoStreams)
        {
            videoStream->SetFFmpegFilters(videoFilters);
        }
    }

    void FFmpegMediaSource::SetFFmpegVideoFilters(hstring const& videoFilters, winrt::FFmpegInteropX::VideoStreamInfo const& videoStream)
    {
        std::lock_guard lock(mutex);
        if (mss == nullptr)
        {
            return;
        }

        for (auto internalvideoStream : videoStreams)
        {
            if (internalvideoStream->VideoInfo() == videoStream)
            {
                internalvideoStream->SetFFmpegFilters(videoFilters);
                break;
            }
        }
    }

    void FFmpegMediaSource::DisableAudioEffects()
    {
        ClearFFmpegAudioFilters();
    }

    void FFmpegMediaSource::ClearFFmpegAudioFilters()
    {
        std::lock_guard lock(mutex);
        if (mss == nullptr)
        {
            return;
        }
        for (auto audioStream : audioStreams)
        {
            audioStream->ClearFFmpegFilters();
        }
    }

    void FFmpegMediaSource::ClearFFmpegAudioFilters(winrt::FFmpegInteropX::AudioStreamInfo const& audioStream)
    {
        std::lock_guard lock(mutex);
        if (mss == nullptr)
        {
            return;
        }

        for (auto internaAudioStream : audioStreams)
        {
            if (internaAudioStream->AudioInfo() == audioStream)
            {
                internaAudioStream->ClearFFmpegFilters();
                break;
            }
        }
    }

    void FFmpegMediaSource::DisableVideoEffects()
    {
        ClearFFmpegVideoFilters();
    }

    void FFmpegMediaSource::ClearFFmpegVideoFilters()
    {
        std::lock_guard lock(mutex);
        if (mss == nullptr)
        {
            return;
        }
        for (auto videoStream : videoStreams)
        {
            videoStream->ClearFFmpegFilters();
        }
    }

    void FFmpegMediaSource::ClearFFmpegVideoFilters(winrt::FFmpegInteropX::VideoStreamInfo const& videoStream)
    {
        std::lock_guard lock(mutex);
        if (mss == nullptr)
        {
            return;
        }

        for (auto internaVideoStream : videoStreams)
        {
            if (internaVideoStream->VideoInfo() == videoStream)
            {
                internaVideoStream->ClearFFmpegFilters();
                break;
            }
        }
    }

    hstring FFmpegMediaSource::GetFFmpegAudioFilters(winrt::FFmpegInteropX::AudioStreamInfo const& audioStream)
    {
        std::lock_guard lock(mutex);
        if (mss == nullptr)
        {
            return hstring{};
        }

        for (int i = 0; i < audioStreams.size(); i++)
        {
            if (audioStreams.at(i)->AudioInfo() == audioStream)
            {
                return audioStreams.at(i)->GetFFmpegFilters();
            }
        }
        return hstring{};
    }

    hstring FFmpegMediaSource::GetFFmpegVideoFilters(winrt::FFmpegInteropX::VideoStreamInfo const& videoStream)
    {
        std::lock_guard lock(mutex);
        if (mss == nullptr)
        {
            return hstring{};
        }
        for (int i = 0; i < videoStreams.size(); i++)
        {
            if (videoStreams.at(i)->VideoInfo() == videoStream)
            {
                return videoStreams.at(i)->GetFFmpegFilters();
            }
        }

        return hstring{};
    }

    FFmpegInteropX::MediaThumbnailData FFmpegMediaSource::ExtractThumbnail()
    {
        if (thumbnailStreamIndex != AVERROR_STREAM_NOT_FOUND)
        {
            // FFmpeg identifies album/cover art from a music file as a video stream
            // Avoid creating unnecessarily video stream from this album/cover art
            if (avFormatCtx->streams[thumbnailStreamIndex]->disposition == AV_DISPOSITION_ATTACHED_PIC)
            {
                auto imageStream = avFormatCtx->streams[thumbnailStreamIndex];
                //save album art to file.
                hstring extension = L".jpeg";
                switch (imageStream->codecpar->codec_id)
                {
                case AV_CODEC_ID_MJPEG:
                case AV_CODEC_ID_MJPEGB:
                case AV_CODEC_ID_JPEG2000:
                case AV_CODEC_ID_JPEGLS: extension = L".jpeg"; break;
                case AV_CODEC_ID_PNG: extension = L".png"; break;
                case AV_CODEC_ID_BMP: extension = L".bmp"; break;
                }

                auto vector = array_view(imageStream->attached_pic.data, imageStream->attached_pic.size);
                DataWriter writer = DataWriter();
                writer.WriteBytes(vector);

                auto retValue = MediaThumbnailData(writer.DetachBuffer(), extension);
                return retValue;
            }
        }

        return nullptr;
    }

    Windows::Media::Core::MediaStreamSource FFmpegMediaSource::GetMediaStreamSource()
    {
        if (this->config->IsFrameGrabber) throw_hresult(E_UNEXPECTED);
        return mss;
    }

    MediaSource FFmpegMediaSource::CreateMediaSource()
    {
        for (auto& stream : sampleProviders)
        {
            if (stream)
            {
                stream->NotifyCreateSource();
            }
        }

        if (this->config->IsFrameGrabber) throw_hresult(E_UNEXPECTED);
        MediaSource source = MediaSource::CreateFromMediaStreamSource(mss);
        for (auto& stream : subtitleStreams)
        {
            source.ExternalTimedMetadataTracks().Append(stream->SubtitleTrack);
        }
        for (auto subtitleInfo : SubtitleStreams())
        {
            if (subtitleInfo.IsExternal()) {
                source.ExternalTimedMetadataTracks().Append(subtitleInfo.SubtitleTrack());
            }
        }

        if (chapterInfos.Size() > 0)
        {
            auto track = TimedMetadataTrack(L"Chapters", L"", TimedMetadataKind::Chapter);
            for (auto chapter : chapterInfos)
            {
                auto cue = ChapterCue();
                cue.Title(chapter.Title());
                cue.StartTime(chapter.StartTime());
                cue.Duration(chapter.Duration());
                track.AddCue(cue);
            }
            source.ExternalTimedMetadataTracks().Append(track);
        }

        return source;
    }

    MediaPlaybackItem FFmpegMediaSource::CreateMediaPlaybackItem()
    {
        std::lock_guard lock(mutex);
        if (mss == nullptr)
        {
            throw_hresult(RO_E_CLOSED);
        }
        if (this->config->IsFrameGrabber || playbackItem != nullptr) throw_hresult(E_UNEXPECTED);
        playbackItem = MediaPlaybackItem(CreateMediaSource());
        InitializePlaybackItem(playbackItem);
        return playbackItem;
    }

    MediaPlaybackItem FFmpegMediaSource::CreateMediaPlaybackItem(TimeSpan const& startTime)
    {
        std::lock_guard lock(mutex);
        if (mss == nullptr)
        {
            throw_hresult(RO_E_CLOSED);
        }
        if (this->config->IsFrameGrabber || playbackItem != nullptr) throw_hresult(E_UNEXPECTED);
        playbackItem = MediaPlaybackItem(CreateMediaSource(), startTime);
        InitializePlaybackItem(playbackItem);
        return playbackItem;
    }

    MediaPlaybackItem FFmpegMediaSource::CreateMediaPlaybackItem(TimeSpan const& startTime, TimeSpan const& durationLimit)
    {
        std::lock_guard lock(mutex);
        if (mss == nullptr)
        {
            throw_hresult(RO_E_CLOSED);
        }
        if (this->config->IsFrameGrabber || playbackItem != nullptr) throw_hresult(E_UNEXPECTED);
        playbackItem = MediaPlaybackItem(CreateMediaSource(), startTime, durationLimit);
        InitializePlaybackItem(playbackItem);
        return playbackItem;
    }

    IAsyncOperation<Collections::IVectorView<FFmpegInteropX::SubtitleStreamInfo>> FFmpegMediaSource::AddExternalSubtitleAsync(IRandomAccessStream stream, hstring streamName)
    {
        auto strong = get_strong();

        winrt::apartment_context caller; // Capture calling context.
        co_await winrt::resume_background();

        auto cancellation = co_await get_cancellation_token();
        auto subConfig(winrt::make_self<MediaSourceConfig>());
        subConfig->IsExternalSubtitleParser = true;
        subConfig->DefaultSubtitleStreamName(streamName);
        subConfig->DefaultSubtitleDelay(this->SubtitleDelay());
        subConfig->AutoCorrectAnsiSubtitles(this->config->AutoCorrectAnsiSubtitles());
        subConfig->AnsiSubtitleEncoding(this->config->AnsiSubtitleEncoding());
        subConfig->OverrideSubtitleStyles(this->config->OverrideSubtitleStyles());
        subConfig->SubtitleRegion(this->config->SubtitleRegion());
        subConfig->SubtitleStyle(this->config->SubtitleStyle());
        subConfig->AutoCorrectAnsiSubtitles(this->config->AutoCorrectAnsiSubtitles());
        subConfig->AutoSelectForcedSubtitles(false);
        subConfig->MinimumSubtitleDuration(this->config->MinimumSubtitleDuration());
        subConfig->AdditionalSubtitleDuration(this->config->AdditionalSubtitleDuration());
        subConfig->PreventModifiedSubtitleDurationOverlap(this->config->PreventModifiedSubtitleDurationOverlap());

        auto videoDescriptor = currentVideoStream ? (currentVideoStream->StreamDescriptor()).as<VideoStreamDescriptor>() : nullptr;
        if (videoDescriptor)
        {
            subConfig->AdditionalFFmpegSubtitleOptions = PropertySet();

            subConfig->AdditionalFFmpegSubtitleOptions.Insert(L"subfps",
                winrt::box_value(winrt::to_hstring(videoDescriptor.EncodingProperties().FrameRate().Numerator()) + L"/" + winrt::to_hstring(videoDescriptor.EncodingProperties().FrameRate().Denominator())));
        }
        auto externalSubsParser = FFmpegMediaSource::CreateFromStream(stream, subConfig, nullptr);

        if (externalSubsParser->SubtitleStreams().Size() > 0)
        {
            if (videoDescriptor)
            {
                auto encodingProperties = videoDescriptor.EncodingProperties();
                auto pixelAspect = (double)encodingProperties.PixelAspectRatio().Numerator() / encodingProperties.PixelAspectRatio().Denominator();
                auto videoAspect = ((double)encodingProperties.Width() / encodingProperties.Height()) / pixelAspect;
                for (auto& subtitleStream : externalSubsParser->subtitleStreams)
                {
                    subtitleStream->NotifyVideoFrameSize(encodingProperties.Width(), encodingProperties.Height(), videoAspect);
                }
            }

            int readResult = 0;
            while ((readResult = externalSubsParser->m_pReader->ReadPacket()) >= 0)
            {
                //Concurrency::interruption_point();
                if (cancellation()) co_return nullptr;
            }
        }

        Collections::IVectorView<FFmpegInteropX::SubtitleStreamInfo> result;
        {
            std::lock_guard lock(mutex);
            if (mss == nullptr)
            {
                throw_hresult(RO_E_CLOSED);
            }
            if (SubtitleDelay().count() != externalSubsParser->SubtitleDelay().count())
            {
                externalSubsParser->SetSubtitleDelay(SubtitleDelay());
            }

            int subtitleTracksCount = 0;

            for (auto& externalSubtitle : externalSubsParser->subtitleStreams)
            {
                if (externalSubtitle->SubtitleTrack.Cues().Size() > 0)
                {
                    // detach stream
                    externalSubtitle->Detach();

                    // find and add stream info
                    for (auto subtitleInfo : externalSubsParser->SubtitleStreams())
                    {
                        if (subtitleInfo.SubtitleTrack() == externalSubtitle->SubtitleTrack)
                        {
                            subtitleStrInfos.Append(subtitleInfo);
                            break;
                        }
                    }

                    // add stream
                    subtitleStreams.push_back(externalSubtitle);
                    if (this->PlaybackItem() != nullptr)
                    {
                        PlaybackItem().Source().ExternalTimedMetadataTracks().Append(externalSubtitle->SubtitleTrack);
                    }
                    subtitleTracksCount++;
                }
            }

            if (subtitleTracksCount == 0)
            {
                throw_hresult(E_INVALIDARG);
            }

            subtitleStreamInfos = subtitleStrInfos.GetView();
            result = externalSubsParser->SubtitleStreams();
        }

        co_await caller;
        co_return result;
    }

    IAsyncOperation<Collections::IVectorView<FFmpegInteropX::SubtitleStreamInfo>> FFmpegMediaSource::AddExternalSubtitleAsync(IRandomAccessStream stream)
    {
        return AddExternalSubtitleAsync(stream, config->DefaultExternalSubtitleStreamName());
    }

    void FFmpegMediaSource::StartBuffering()
    {
        std::lock_guard lock(mutex);
        if (mss == nullptr)
        {
            return;
        }
        m_pReader->Start();
    }

    FFmpegInteropX::MediaSourceConfig FFmpegMediaSource::Configuration()
    {
        return config.as<winrt::FFmpegInteropX::MediaSourceConfig>();
    }

    Collections::IMapView<hstring, Collections::IVectorView<hstring>> FFmpegMediaSource::MetadataTags()
    {
        std::lock_guard lock(mutex);
        if (mss == nullptr)
        {
            throw_hresult(RO_E_CLOSED);
        }
        metadata->LoadMetadataTags(avFormatCtx);
        return metadata->MetadataTags();
    }

    TimeSpan FFmpegMediaSource::Duration()
    {
        return mediaDuration;
    }

    FFmpegInteropX::VideoStreamInfo FFmpegMediaSource::CurrentVideoStream()
    {
        auto stream = currentVideoStream;
        return stream ? stream->VideoInfo() : nullptr;
    }

    FFmpegInteropX::AudioStreamInfo FFmpegMediaSource::CurrentAudioStream()
    {
        auto stream = currentAudioStream;
        return stream ? stream->AudioInfo() : nullptr;
    }

    Collections::IVectorView<FFmpegInteropX::VideoStreamInfo> FFmpegMediaSource::VideoStreams()
    {
        return videoStreamInfos;
    }

    Collections::IVectorView<FFmpegInteropX::AudioStreamInfo> FFmpegMediaSource::AudioStreams()
    {
        return audioStreamInfos;
    }

    Collections::IVectorView<FFmpegInteropX::SubtitleStreamInfo> FFmpegMediaSource::SubtitleStreams()
    {
        return subtitleStreamInfos;
    }

    Collections::IVectorView<FFmpegInteropX::ChapterInfo> FFmpegMediaSource::ChapterInfos()
    {
        return chapterInfos;
    }

    FFmpegInteropX::FormatInfo FFmpegMediaSource::FormatInfo()
    {
        return formatInfo;
    }

    bool FFmpegMediaSource::HasThumbnail()
    {
        return thumbnailStreamIndex != AVERROR_STREAM_NOT_FOUND;
    }

    MediaPlaybackItem FFmpegMediaSource::PlaybackItem()
    {
        return playbackItem;
    }

    TimeSpan FFmpegMediaSource::SubtitleDelay()
    {
        return subtitleDelay;
    }

    TimeSpan FFmpegMediaSource::BufferTime()
    {
        std::lock_guard lock(mutex);
        if (mss == nullptr)
        {
            throw_hresult(RO_E_CLOSED);
        }
        return mss.BufferTime();
    }

    void FFmpegMediaSource::BufferTime(TimeSpan const& value)
    {
        std::lock_guard lock(mutex);
        if (mss == nullptr)
        {
            return;
        }
        mss.BufferTime(value);
    }

    void FFmpegMediaSource::SetStreamDelay(FFmpegInteropX::IStreamInfo const& stream, TimeSpan const& delay)
    {
        for (auto provider : sampleProviders)
        {
            if (provider->StreamInfo() == stream)
            {
                provider->SetStreamDelay(delay.count());
                return;
            }
        }
    }

    TimeSpan FFmpegMediaSource::GetStreamDelay(FFmpegInteropX::IStreamInfo const& stream)
    {
        for (auto provider : sampleProviders)
        {
            if (provider->StreamInfo() == stream)
            {
                return TimeSpan{ provider->GetStreamDelay() };
            }
        }

        return TimeSpan{ 0L };
    }

    MediaPlaybackSession FFmpegMediaSource::PlaybackSession()
    {
        return session;
    }

    void FFmpegMediaSource::PlaybackSession(MediaPlaybackSession const& value)
    {
        std::lock_guard lock(mutex);
        if (session)
        {
            session.PositionChanged(sessionPositionEvent);
        }
        session = value;
        if (value)
        {
            sessionPositionEvent = value.PositionChanged({ get_weak(), &FFmpegInteropX::implementation::FFmpegMediaSource::OnPositionChanged });
        }
    }

    void FFmpegMediaSource::Close()
    {
        isShuttingDown = true;

        std::lock_guard lock(mutex);
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
            m_pReader->Stop();
            m_pReader->Flush();
            m_pReader.reset();;
        }

        for (auto& x : subtitleStreams)
            x.reset();
        for (auto& x : sampleProviders)
            x.reset();
        for (auto& x : audioStreams)
            x.reset();
        for (auto& x : videoStreams)
            x.reset();

        subtitleStreams.clear();
        sampleProviders.clear();
        audioStreams.clear();
        videoStreams.clear();

        if (avFormatCtx)
            avformat_close_input(&avFormatCtx);
        if (avIOCtx)
        {
            avIOCtx->opaque = NULL;
            avio_closep(&avIOCtx);
        }
        if (avDict)
            av_dict_free(&avDict);

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

        device = nullptr;
        deviceContext = nullptr;
        deviceManager = nullptr;

        if (PlaybackSession())
        {
            PlaybackSession().PositionChanged(sessionPositionEvent);
            PlaybackSession(nullptr);
        }
    }


    std::shared_ptr<MediaSampleProvider> FFmpegMediaSource::CreateAudioSampleProvider(AVStream* avStream, AVCodecContext* avAudioCodecCtx, int index)
    {
        UNREFERENCED_PARAMETER(avStream);
        std::shared_ptr<MediaSampleProvider> audioSampleProvider = nullptr;
        if (avAudioCodecCtx->codec_id == AV_CODEC_ID_AAC && config->PassthroughAudioAAC())
        {
            AudioEncodingProperties encodingProperties;
            if (avAudioCodecCtx->extradata_size == 0)
            {
                encodingProperties = AudioEncodingProperties::CreateAacAdts(avAudioCodecCtx->sample_rate, avAudioCodecCtx->channels, (unsigned int)avAudioCodecCtx->bit_rate);
            }
            else
            {
                encodingProperties = AudioEncodingProperties::CreateAac(avAudioCodecCtx->profile == FF_PROFILE_AAC_HE || avAudioCodecCtx->profile == FF_PROFILE_AAC_HE_V2 ? avAudioCodecCtx->sample_rate / 2 : avAudioCodecCtx->sample_rate, avAudioCodecCtx->channels, (unsigned int)avAudioCodecCtx->bit_rate);
            }
            audioSampleProvider = std::shared_ptr<MediaSampleProvider>(new CompressedSampleProvider(m_pReader, avFormatCtx, avAudioCodecCtx, config.as<winrt::FFmpegInteropX::MediaSourceConfig>(), index, encodingProperties, HardwareDecoderStatus::Unknown));
        }
        else if (avAudioCodecCtx->codec_id == AV_CODEC_ID_MP3 && config->PassthroughAudioMP3())
        {
            AudioEncodingProperties encodingProperties = AudioEncodingProperties::CreateMp3(avAudioCodecCtx->sample_rate, avAudioCodecCtx->channels, (unsigned int)avAudioCodecCtx->bit_rate);
            audioSampleProvider = std::shared_ptr<MediaSampleProvider>(new CompressedSampleProvider(m_pReader, avFormatCtx, avAudioCodecCtx, config.as<winrt::FFmpegInteropX::MediaSourceConfig>(), index, encodingProperties, HardwareDecoderStatus::Unknown));
        }
        else
        {
            audioSampleProvider = std::shared_ptr<MediaSampleProvider>(new UncompressedAudioSampleProvider(m_pReader, avFormatCtx, avAudioCodecCtx, config.as<winrt::FFmpegInteropX::MediaSourceConfig>(), index));
        }

        auto hr = audioSampleProvider->Initialize();
        if (FAILED(hr))
        {
            audioSampleProvider = nullptr;
        }

        return audioSampleProvider;
    }

    bool FFmpegMediaSource::CheckUseHardwareAcceleration(AVCodecContext* avCodecCtx, HardwareAccelerationStatus const& status, HardwareDecoderStatus& hardwareDecoderStatus, int maxProfile, int maxLevel)
    {
        UNREFERENCED_PARAMETER(maxProfile);
        bool result = false;
        if (!config->IsFrameGrabber)
        {
#pragma warning (disable: 4973)

            if (config->VideoDecoderMode() == VideoDecoderMode::AutomaticSystemDecoder)
            {
                result = CodecChecker::CheckUseHardwareAcceleration(status,
                    avCodecCtx->codec_id, avCodecCtx->profile, avCodecCtx->width, avCodecCtx->height);

                // check level, if restricted
                if (result && maxLevel >= 0)
                {
                    result = avCodecCtx->level <= maxLevel;
                }

                hardwareDecoderStatus = result ? HardwareDecoderStatus::Available : HardwareDecoderStatus::NotAvailable;
            }
            else if (config->VideoDecoderMode() == VideoDecoderMode::ForceSystemDecoder)
            {
                result = true;
            }
            else
            {
                result = false;
            }
#pragma warning (default: 4973)

        }

        return result;
    }

    void FFmpegMediaSource::CheckUseHdr(winrt::com_ptr<MediaSourceConfig> const& config)
    {
        bool useHdr = false;
        switch (config->HdrSupport())
        {
        case HdrSupport::Enabled:
            useHdr = true;
            break;
        case HdrSupport::Automatic:
            try
            {
                auto displayInfo = Windows::Graphics::Display::DisplayInformation::GetForCurrentView();
                if (displayInfo)
                {
                    auto colorInfo = displayInfo.GetAdvancedColorInfo();
                    if (colorInfo.CurrentAdvancedColorKind() == Windows::Graphics::Display::AdvancedColorKind::HighDynamicRange)
                    {
                        useHdr = true;
                    }
                }
            }
            catch (...)
            {
            }
            break;
        default:
            useHdr = false;
        }
        config->ApplyHdrColorInfo = useHdr;
    }

    std::shared_ptr<MediaSampleProvider> FFmpegMediaSource::CreateVideoSampleProvider(AVStream* avStream, AVCodecContext* avVideoCodecCtx, int index)
    {
        UNREFERENCED_PARAMETER(avStream);

        std::shared_ptr<MediaSampleProvider> videoSampleProvider = nullptr;
        winrt::FFmpegInteropX::HardwareDecoderStatus hardwareDecoderStatus = HardwareDecoderStatus::Unknown;

#pragma warning (disable: 4973)

        if (config->VideoDecoderMode() == VideoDecoderMode::AutomaticSystemDecoder)
        {
            CodecChecker::Initialize();
        }

        if (avVideoCodecCtx->codec_id == AV_CODEC_ID_H264 &&
            (CheckUseHardwareAcceleration(avVideoCodecCtx, CodecChecker::HardwareAccelerationH264(), hardwareDecoderStatus, config->SystemDecoderH264MaxProfile(), config->SystemDecoderH264MaxLevel())))
        {
            auto videoProperties = VideoEncodingProperties::CreateH264();

            // Check for H264 bitstream flavor. H.264 AVC extradata starts with 1 while non AVC one starts with 0
            if (avVideoCodecCtx->extradata != nullptr && avVideoCodecCtx->extradata_size > 0 && avVideoCodecCtx->extradata[0] == 1)
            {
                videoSampleProvider = std::shared_ptr<MediaSampleProvider>(new H264AVCSampleProvider(m_pReader, avFormatCtx, avVideoCodecCtx, config.as<winrt::FFmpegInteropX::MediaSourceConfig>(), index, videoProperties, hardwareDecoderStatus));
            }
            else
            {
                videoSampleProvider = std::shared_ptr<MediaSampleProvider>(new NALPacketSampleProvider(m_pReader, avFormatCtx, avVideoCodecCtx, config.as<winrt::FFmpegInteropX::MediaSourceConfig>(), index, videoProperties, hardwareDecoderStatus));
            }
        }
        else if (avVideoCodecCtx->codec_id == AV_CODEC_ID_HEVC &&
            CheckUseHardwareAcceleration(avVideoCodecCtx, CodecChecker::HardwareAccelerationHEVC(), hardwareDecoderStatus, config->SystemDecoderHEVCMaxProfile(), config->SystemDecoderHEVCMaxLevel()))
        {
            auto videoProperties = VideoEncodingProperties::CreateHevc();

            // Check for HEVC bitstream flavor.
            if (avVideoCodecCtx->extradata != nullptr && avVideoCodecCtx->extradata_size > 22 &&
                (avVideoCodecCtx->extradata[0] || avVideoCodecCtx->extradata[1] || avVideoCodecCtx->extradata[2] > 1))
            {
                videoSampleProvider = std::shared_ptr<MediaSampleProvider>(new HEVCSampleProvider(m_pReader, avFormatCtx, avVideoCodecCtx, config.as<winrt::FFmpegInteropX::MediaSourceConfig>(), index, videoProperties, hardwareDecoderStatus));
            }
            else
            {
                videoSampleProvider = std::shared_ptr<MediaSampleProvider>(new NALPacketSampleProvider(m_pReader, avFormatCtx, avVideoCodecCtx, config.as<winrt::FFmpegInteropX::MediaSourceConfig>(), index, videoProperties, hardwareDecoderStatus));
            }
        }
        else if (avVideoCodecCtx->codec_id == AV_CODEC_ID_WMV3 &&
            CheckUseHardwareAcceleration(avVideoCodecCtx, CodecChecker::HardwareAccelerationWMV3(), hardwareDecoderStatus, -1, -1) &&
            avVideoCodecCtx->extradata_size > 0)
        {
            auto videoProperties = VideoEncodingProperties();
            videoProperties.Subtype(MediaEncodingSubtypes::Wmv3());

            auto extradata = array_view(avVideoCodecCtx->extradata, avVideoCodecCtx->extradata_size);
            videoProperties.SetFormatUserData(extradata);
            videoSampleProvider = std::shared_ptr<MediaSampleProvider>(new CompressedSampleProvider(m_pReader, avFormatCtx, avVideoCodecCtx, config.as<winrt::FFmpegInteropX::MediaSourceConfig>(), index, videoProperties, hardwareDecoderStatus));
        }
        else if (avVideoCodecCtx->codec_id == AV_CODEC_ID_VC1 &&
            CheckUseHardwareAcceleration(avVideoCodecCtx, CodecChecker::HardwareAccelerationVC1(), hardwareDecoderStatus, -1, -1) &&
            avVideoCodecCtx->extradata_size > 0)
        {
            auto videoProperties = VideoEncodingProperties();
            videoProperties.Subtype(MediaEncodingSubtypes::Wvc1());

            auto extradata = array_view(avVideoCodecCtx->extradata, avVideoCodecCtx->extradata_size);
            videoProperties.SetFormatUserData(extradata);
            videoSampleProvider = std::shared_ptr<MediaSampleProvider>(new CompressedSampleProvider(m_pReader, avFormatCtx, avVideoCodecCtx, config.as<winrt::FFmpegInteropX::MediaSourceConfig>(), index, videoProperties, hardwareDecoderStatus));
        }
        else if (avVideoCodecCtx->codec_id == AV_CODEC_ID_MPEG2VIDEO &&
            CheckUseHardwareAcceleration(avVideoCodecCtx, CodecChecker::HardwareAccelerationMPEG2(), hardwareDecoderStatus, -1, -1))
        {
            auto videoProperties = VideoEncodingProperties();
            videoProperties.Subtype(MediaEncodingSubtypes::Mpeg2());

            videoSampleProvider = std::shared_ptr<MediaSampleProvider>(new CompressedSampleProvider(m_pReader, avFormatCtx, avVideoCodecCtx, config.as<winrt::FFmpegInteropX::MediaSourceConfig>(), index, videoProperties, hardwareDecoderStatus));
        }
        else if (avVideoCodecCtx->codec_id == AV_CODEC_ID_VP9 &&
            CheckUseHardwareAcceleration(avVideoCodecCtx, CodecChecker::HardwareAccelerationVP9(), hardwareDecoderStatus, -1, -1))
        {
            auto videoProperties = VideoEncodingProperties();
            videoProperties.Subtype(MediaEncodingSubtypes::Vp9());

            videoSampleProvider = std::shared_ptr<MediaSampleProvider>(new CompressedSampleProvider(m_pReader, avFormatCtx, avVideoCodecCtx, config.as<winrt::FFmpegInteropX::MediaSourceConfig>(), index, videoProperties, hardwareDecoderStatus));
        }
        else if (avVideoCodecCtx->codec_id == AV_CODEC_ID_VP8 &&
            CheckUseHardwareAcceleration(avVideoCodecCtx, CodecChecker::HardwareAccelerationVP8(), hardwareDecoderStatus, -1, -1))

        {
            auto videoProperties = VideoEncodingProperties();
            videoProperties.Subtype(Windows::Media::Core::CodecSubtypes::VideoFormatVP80());

            videoSampleProvider = std::shared_ptr<MediaSampleProvider>(new CompressedSampleProvider(m_pReader, avFormatCtx, avVideoCodecCtx, config.as<winrt::FFmpegInteropX::MediaSourceConfig>(), index, videoProperties, hardwareDecoderStatus));
        }
        else if (avVideoCodecCtx->hw_device_ctx)
        {
            hardwareDecoderStatus = HardwareDecoderStatus::Available;
            videoSampleProvider = std::shared_ptr<MediaSampleProvider>(new D3D11VideoSampleProvider(m_pReader, avFormatCtx, avVideoCodecCtx, config.as<winrt::FFmpegInteropX::MediaSourceConfig>(), index, hardwareDecoderStatus, config->ApplyHdrColorInfo));
        }
        else
        {
            if (config->VideoDecoderMode() == VideoDecoderMode::AutomaticSystemDecoder)
            {
                hardwareDecoderStatus = HardwareDecoderStatus::NotAvailable;
            }
            videoSampleProvider = std::shared_ptr<MediaSampleProvider>(new UncompressedVideoSampleProvider(m_pReader, avFormatCtx, avVideoCodecCtx, config.as<winrt::FFmpegInteropX::MediaSourceConfig>(), index, hardwareDecoderStatus, config->ApplyHdrColorInfo));
        }

#pragma warning (default: 4973)

        auto hr = videoSampleProvider->Initialize();

        if (FAILED(hr))
        {
            videoSampleProvider = nullptr;
        }

        return videoSampleProvider;
    }

    HRESULT FFmpegMediaSource::ParseOptions(PropertySet const& ffmpegOptions)
    {
        HRESULT hr = S_OK;

        // Convert FFmpeg options given in PropertySet to AVDictionary. List of options can be found in https://www.ffmpeg.org/ffmpeg-protocols.html
        if (ffmpegOptions != nullptr)
        {
            auto options = ffmpegOptions.First();

            while (options.HasCurrent())
            {
                auto key = StringUtils::PlatformStringToUtf8String(options.Current().Key());
                hstring value = StringUtils::ToString(options.Current().Value());
                if (!value.empty())
                {
                    // Add key and value pair entry
                    auto valCstr = StringUtils::PlatformStringToUtf8String(value);
                    if (av_dict_set(&avDict, key.c_str(), valCstr.c_str(), 0) < 0)
                    {
                        hr = E_INVALIDARG;
                        break;
                    }
                }
                options.MoveNext();
            }
        }

        return hr;
    }

    void FFmpegMediaSource::OnStarting(MediaStreamSource const& sender, MediaStreamSourceStartingEventArgs const& args)
    {
        std::lock_guard lock(mutex);
        if (mss == nullptr)
        {
            return;
        }
        MediaStreamSourceStartingRequest request = args.Request();

        try
        {
            if (isFirstSeek && avHardwareContext)
            {
                HRESULT hr = DirectXInteropHelper::GetDeviceManagerFromStreamSource(sender, deviceManager);
                if (SUCCEEDED(hr))
                    hr = D3D11VideoSampleProvider::InitializeHardwareDeviceContext(sender, avHardwareContext, device, deviceContext, deviceManager, &deviceHandle);

                if (SUCCEEDED(hr))
                {
                    // assign device and context
                    for (auto& stream : videoStreams)
                    {
                        // set device pointers to stream
                        hr = stream->SetHardwareDevice(device, deviceContext, avHardwareContext);

                        if (!SUCCEEDED(hr))
                        {
                            break;
                        }
                    }
                }
                else
                {
                    // unref all hw device contexts
                    for (auto& stream : videoStreams)
                    {
                        stream->FreeHardwareDevice();
                    }
                    av_buffer_unref(&avHardwareContext);
                    device = nullptr;
                    deviceContext = nullptr;
                }
            }

            // Perform seek operation when MediaStreamSource received seek event from MediaElement
            if (request.StartPosition() && request.StartPosition().Value().count() <= mediaDuration.count() && (!isFirstSeek || request.StartPosition().Value().count() > 0))
            {
                if (currentVideoStream && !currentVideoStream->IsEnabled())
                {
                    currentVideoStream->EnableStream();
                }

                if (currentAudioStream && !currentAudioStream->IsEnabled())
                {
                    currentAudioStream->EnableStream();
                }

                TimeSpan actualPosition = request.StartPosition().Value();
                auto hr = Seek(request.StartPosition().Value(), actualPosition, true);
                if (SUCCEEDED(hr))
                {
                    request.SetActualStartPosition(actualPosition);
                }
            }
        }
        catch (...)
        {
            DebugMessage(L"Exception in OnStarting()!");
        }

        isFirstSeek = false;
        isFirstSeekAfterStreamSwitch = false;
    }

    void FFmpegMediaSource::OnSampleRequested(winrt::Windows::Media::Core::MediaStreamSource const& sender, winrt::Windows::Media::Core::MediaStreamSourceSampleRequestedEventArgs const& args)
    {
        UNREFERENCED_PARAMETER(sender);
        std::lock_guard lock(mutex);
        if (mss == nullptr)
        {
            return;
        }

        try
        {
            if (config->ReadAheadBufferEnabled())
            {
                m_pReader->Start();
            }
            if (currentAudioStream && args.Request().StreamDescriptor() == currentAudioStream->StreamDescriptor())
            {
                auto sample = currentAudioStream->GetNextSample();
                CheckExtendDuration(sample);
                args.Request().Sample(sample);
            }
            else if (currentVideoStream && args.Request().StreamDescriptor() == currentVideoStream->StreamDescriptor())
            {
                CheckVideoDeviceChanged();
                auto sample = currentVideoStream->GetNextSample();
                CheckExtendDuration(sample);
                args.Request().Sample(sample);
            }
            else
            {
                args.Request().Sample(nullptr);
            }
        }
        catch (...)
        {
            DebugMessage(L"Exception in OnSampleRequested()!");
        }
    }

    void FFmpegMediaSource::CheckExtendDuration(MediaStreamSample sample)
    {
        if (sample && config->AutoExtendDuration())
        {
            auto sampleEnd = sample.Timestamp() + sample.Duration();
            if (TimeSpan::zero() < mediaDuration && (mediaDuration < sampleEnd ||
                (lastDurationExtension && (mediaDuration - sampleEnd).count() < 5000000)))
            {
                auto extension = min(lastDurationExtension + 1, 5);

                mediaDuration += TimeSpan{ extension * 10000000 };
                mss.Duration(mediaDuration);

                lastDurationExtension = extension;
            }
        }
    }

    void FFmpegMediaSource::CheckVideoDeviceChanged()
    {
        bool hasDeviceChanged = false;
        HRESULT hr = S_OK;
        if (deviceManager)
        {
            hr = deviceManager->TestDevice(deviceHandle);
            hasDeviceChanged = hr == MF_E_DXGI_NEW_VIDEO_DEVICE;
        }

        if (hasDeviceChanged && avHardwareContext)
        {
            hr = S_OK;
            av_buffer_unref(&avHardwareContext);
            device = nullptr;
            deviceContext = nullptr;

            if (deviceHandle && deviceManager)
                deviceManager->CloseDeviceHandle(deviceHandle);

            avHardwareContext = av_hwdevice_ctx_alloc(AVHWDeviceType::AV_HWDEVICE_TYPE_D3D11VA);

            if (!avHardwareContext)
            {
                hr = E_OUTOFMEMORY;
            }

            if (SUCCEEDED(hr))
            {
                hr = D3D11VideoSampleProvider::InitializeHardwareDeviceContext(mss, avHardwareContext, device, deviceContext, deviceManager, &deviceHandle);
            }

            if (SUCCEEDED(hr))
            {
                // assign device and context
                for (auto& stream : videoStreams)
                {
                    // set device pointers to stream
                    hr = stream->SetHardwareDevice(device, deviceContext, avHardwareContext);

                    if (!SUCCEEDED(hr))
                    {
                        break;
                    }
                }
            }

            if (SUCCEEDED(hr))
            {
                if (mss.CanSeek())
                {
                    // seek to last keyframe position
                    TimeSpan lastVideoTimestamp = currentVideoStream->LastSampleTimestamp;
                    TimeSpan actualPosition;
                    Seek(lastVideoTimestamp, actualPosition, false);

                    // decode video until we are at target position
                    while (true)
                    {
                        auto sample = currentVideoStream->GetNextSample();
                        if (!sample || sample.Timestamp() >= lastVideoTimestamp)
                        {
                            break;
                        }
                    }

                    // decode audio until we are at target position
                    if (currentAudioStream)
                    {
                        TimeSpan lastAudioTimestamp = currentAudioStream->LastSampleTimestamp;
                        while (true)
                        {
                            auto sample = currentAudioStream->GetNextSample();
                            if (!sample || sample.Timestamp() >= lastAudioTimestamp)
                            {
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    void FFmpegMediaSource::OnSwitchStreamsRequested(MediaStreamSource const& sender, MediaStreamSourceSwitchStreamsRequestedEventArgs const& args)
    {
        UNREFERENCED_PARAMETER(sender);
        std::lock_guard lock(mutex);
        if (mss == nullptr)
        {
            return;
        }

        try
        {
            m_pReader->Stop();
            m_pReader->Flush();

            if (currentAudioStream && args.Request().OldStreamDescriptor() == currentAudioStream->StreamDescriptor())
            {
                currentAudioStream->DisableStream();
                currentAudioStream = nullptr;
            }
            if (currentVideoStream && args.Request().OldStreamDescriptor() == currentVideoStream->StreamDescriptor())
            {
                currentVideoStream->DisableStream();
                currentVideoStream = nullptr;
            }

            for (auto& stream : audioStreams)
            {
                if (stream->StreamDescriptor() == args.Request().NewStreamDescriptor())
                {
                    currentAudioStream = stream;
                    currentAudioStream->EnableStream();
                }
            }
            for (auto& stream : videoStreams)
            {
                if (stream->StreamDescriptor() == args.Request().NewStreamDescriptor())
                {
                    currentVideoStream = stream;
                    currentVideoStream->EnableStream();
                }
            }

            isFirstSeekAfterStreamSwitch = config->FastSeekSmartStreamSwitching();
        }
        catch (...)
        {
            DebugMessage(L"Exception in OnSwitchStreamsRequested()!");
        }
    }

    HRESULT FFmpegMediaSource::Seek(const TimeSpan& position, TimeSpan& actualPosition, bool allowFastSeek)
    {
        DebugMessage(L"Seek\n");

        auto diffCurrent = position - currentPosition;
        auto diffLast = position - lastPosition;
        bool isSeekBeforeStreamSwitch = allowFastSeek && config->FastSeekSmartStreamSwitching() && !isFirstSeekAfterStreamSwitch && diffCurrent.count() > 0 && diffCurrent.count() < 5000000 && diffLast.count() > 0 && diffLast.count() < 10000000;

        bool fastSeek = allowFastSeek && config->FastSeek() && currentVideoStream && PlaybackSession() && !isFirstSeekAfterStreamSwitch;
        if (isSeekBeforeStreamSwitch)
        {
            return S_OK;
        }
        else if (position == currentPosition && position == lastPosition && position == lastSeek && !isFirstSeekAfterStreamSwitch && position.count() > 0)
        {
            DebugMessage(L"Skipping double seek request.\n");
            return S_OK;
        }
        else
        {
            lastSeek = position;
            return m_pReader->Seek(position, actualPosition, lastPosition, fastSeek, currentVideoStream, currentAudioStream);
        }
    }

    void FFmpegMediaSource::OnPositionChanged(MediaPlaybackSession const& sender, IInspectable const& args)
    {
        UNREFERENCED_PARAMETER(sender);
        UNREFERENCED_PARAMETER(args);
        std::lock_guard lock(mutex);
        if (mss == nullptr)
        {
            return;
        }
        lastPosition = currentPosition;
        currentPosition = sender.Position();
    }

    // Static functions passed to FFmpeg
    static int FileStreamRead(void* ptr, uint8_t* buf, int bufSize)
    {
        FFmpegMediaSource* mss = reinterpret_cast<FFmpegMediaSource*>(ptr);
        ULONG bytesRead = 0;
        HRESULT hr = mss->fileStreamData->Read(buf, bufSize, &bytesRead);

        if (FAILED(hr))
        {
            return -1;
        }

        // Check beginning of file for BOM on first read
        if (mss->streamByteOrderMark == ByteOrderMark::Unchecked)
        {
            if (bytesRead >= 4)
            {
                auto bom = ((uint32_t*)buf)[0];
                if ((bom & 0x00FFFFFF) == 0x00BFBBEF)
                {
                    mss->streamByteOrderMark = ByteOrderMark::UTF8;
                }
                else
                {
                    mss->streamByteOrderMark = ByteOrderMark::Unknown;
                }
            }
            else
            {
                mss->streamByteOrderMark = ByteOrderMark::Unknown;
            }
        }

        // If we succeed but don't have any bytes, assume end of file
        if (bytesRead == 0)
        {
            return AVERROR_EOF;  // Let FFmpeg know that we have reached eof
        }

        return bytesRead;
    }

    // Static function to seek in file stream. Credit to Philipp Sch http://www.codeproject.com/Tips/489450/Creating-Custom-FFmpeg-IO-Context
    static int64_t FileStreamSeek(void* ptr, int64_t pos, int whence)
    {
        FFmpegMediaSource* mss = reinterpret_cast<FFmpegMediaSource*>(ptr);
        if (whence == AVSEEK_SIZE)
        {
            // get stream size
            STATSTG status;
            if (FAILED(mss->fileStreamData->Stat(&status, STATFLAG_NONAME)))
            {
                return -1;
            }
            return status.cbSize.QuadPart;
        }
        else
        {
            LARGE_INTEGER in;
            in.QuadPart = pos;
            ULARGE_INTEGER out = { 0 };

            if (FAILED(mss->fileStreamData->Seek(in, whence, &out)))
            {
                return -1;
            }

            return out.QuadPart; // Return the new position:
        }
    }

    static int IsShuttingDown(void* ptr)
    {
        FFmpegMediaSource* mss = reinterpret_cast<FFmpegMediaSource*>(ptr);
        if (mss->isShuttingDown)
        {
            return 1;
        }
        return 0;
    }
}
