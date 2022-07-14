#pragma once
#include "pch.h"
#include <winrt/Windows.Media.Core.h>
#include "CompressedSampleProvider.h"
#include "NativeBufferFactory.h"
#include "ReferenceCue.h"
#include "AvCodecContextHelpers.h"
#include <winrt/FFmpegInteropX.h>

namespace FFmpegInteropX
{
    using namespace winrt::Windows::UI::Core;
    using namespace winrt::Windows::Media::Playback;
    using namespace winrt::Windows::Media::Core;
    using namespace winrt::Windows::Foundation;

    class SubtitleProvider :
        public CompressedSampleProvider, public std::enable_shared_from_this<SubtitleProvider>
    {
    public:
        TimedMetadataTrack SubtitleTrack = { nullptr };

        MediaPlaybackItem PlaybackItem = { nullptr };

        TimeSpan SubtitleDelay{};

        SubtitleProvider(std::shared_ptr<FFmpegReader> reader,
            AVFormatContext* avFormatCtx,
            AVCodecContext* avCodecCtx,
            MediaSourceConfig const& config,
            int index,
            TimedMetadataKind const& ptimedMetadataKind,
            winrt::Windows::UI::Core::CoreDispatcher const& pdispatcher)
            : CompressedSampleProvider(reader,
                avFormatCtx,
                avCodecCtx,
                config,
                index,
                HardwareDecoderStatus::Unknown),
            dispatcher(pdispatcher),
            timedMetadataKind(ptimedMetadataKind)
        {
        }

        virtual HRESULT Initialize() override
        {
            InitializeNameLanguageCodec();

            SubtitleTrack = TimedMetadataTrack(Name, Language, timedMetadataKind);
            SubtitleTrack.Label(!Name.empty() ? Name : Language);

            if (!m_config.as<implementation::MediaSourceConfig>()->IsExternalSubtitleParser)
            {
                if (Metadata::ApiInformation::IsEnumNamedValuePresent(L"Windows.Media.Core.TimedMetadataKind", L"ImageSubtitle") &&
                    timedMetadataKind == TimedMetadataKind::ImageSubtitle)
                {
                    SubtitleTrack.CueEntered(weak_handler(this, &SubtitleProvider::OnCueEntered));
                }
                SubtitleTrack.TrackFailed(weak_handler(this, &SubtitleProvider::OnTrackFailed));
            }

            InitializeStreamInfo();

            return S_OK;
        }

    public:

        virtual void InitializeStreamInfo() override
        {
            auto forced = (m_pAvStream->disposition & AV_DISPOSITION_FORCED) == AV_DISPOSITION_FORCED;

            streamInfo = SubtitleStreamInfo(Name, Language, CodecName, (StreamDisposition)m_pAvStream->disposition,
                false, forced, SubtitleTrack, m_config.as<implementation::MediaSourceConfig>()->IsExternalSubtitleParser);
        }

        virtual void NotifyVideoFrameSize(int width, int height, double aspectRatio)
        {
            UNREFERENCED_PARAMETER(width);
            UNREFERENCED_PARAMETER(height);
            UNREFERENCED_PARAMETER(aspectRatio);
        }

        virtual IMediaCue CreateCue(AVPacket* packet, TimeSpan* position, TimeSpan* duration) = 0;

        virtual void QueuePacket(AVPacket* packet) override
        {
            if (m_isEnabled)
            {
                try
                {
                    TimeSpan position = ConvertPosition(packet->pts);
                    TimeSpan duration = ConvertDuration(packet->duration);

                    auto cue = CreateCue(packet, &position, &duration);
                    if (cue && position.count() >= 0)
                    {
                        // apply subtitle delay
                        position += SubtitleDelay;
                        if (position.count() < 0)
                        {
                            negativePositionCues.emplace_back(cue, position.count());
                            position = std::chrono::seconds(0);
                        }

                        // clip previous extended duration cue, if there is one
                        if (lastExtendedDurationCue && m_config.PreventModifiedSubtitleDurationOverlap() &&
                            lastExtendedDurationCue.StartTime() + lastExtendedDurationCue.Duration() > position)
                        {
                            auto diff = position - (lastExtendedDurationCue.StartTime() + lastExtendedDurationCue.Duration());
                            auto newDuration = lastExtendedDurationCue.Duration() + diff;
                            if (newDuration.count() > 0)
                            {
                                lastExtendedDurationCue.Duration() = newDuration;
                                if (!m_config.as<implementation::MediaSourceConfig>().get()->IsExternalSubtitleParser)
                                {
                                    pendingChangedDurationCues.push_back(lastExtendedDurationCue);
                                }
                            }
                            else
                            {
                                // weird subtitle timings, just leave it as is
                            }
                        }

                        lastExtendedDurationCue = nullptr;

                        if (duration.count() < 0)
                        {
                            duration = TimeSpan(InfiniteDuration);
                        }
                        else
                        {
                            if (m_config.AdditionalSubtitleDuration().count() != 0)
                            {
                                duration += m_config.AdditionalSubtitleDuration();
                                lastExtendedDurationCue = cue;
                            }
                            if (duration < m_config.MinimumSubtitleDuration())
                            {
                                duration = m_config.MinimumSubtitleDuration();
                                lastExtendedDurationCue = cue;
                            }
                        }

                        cue.StartTime(position);
                        cue.Duration(duration);
                        AddCue(cue);

                        if (!m_config.as<implementation::MediaSourceConfig>()->IsExternalSubtitleParser)
                        {
                            isPreviousCueInfiniteDuration = duration.count() >= InfiniteDuration;
                        }
                        else
                        {
                            // fixup infinite duration cues for external subs
                            if (isPreviousCueInfiniteDuration)
                            {
                                infiniteDurationCue.Duration(TimeSpan(cue.StartTime() - infiniteDurationCue.StartTime()));
                            }

                            if (duration.count() >= InfiniteDuration)
                            {
                                isPreviousCueInfiniteDuration = true;
                                infiniteDurationCue = cue;
                            }
                            else
                            {
                                isPreviousCueInfiniteDuration = false;
                                infiniteDurationCue = nullptr;
                            }
                        }
                    }
                }
                catch (...)
                {
                    OutputDebugString(L"Failed to create subtitle cue.");
                }
            }
            av_packet_free(&packet);
        }

        void SetSubtitleDelay(TimeSpan const& delay)
        {
            mutex.lock();
            newDelay = delay;
            try
            {
                TriggerUpdateCues();
            }
            catch (...)
            {

            }
            mutex.unlock();
        }

        int parseInt(std::wstring const& str)
        {
            return std::stoi(str, nullptr, 10);
        }

        double parseDouble(std::wstring const& str)
        {
            return std::stod(str);
        }

        int parseHexInt(std::wstring const& str)
        {
            return std::stoi(str, nullptr, 16);
        }

        int parseHexOrDecimalInt(std::wstring const& str, size_t offset)
        {
            if (str.length() > offset + 1 && str[offset] == L'H')
            {
                return parseHexInt(str.substr(offset + 1));
            }
            return parseInt(str.substr(offset));
        }

        bool checkTag(std::wstring const& str, std::wstring const& prefix, size_t minParamLenth = 1)
        {
            return
                str.size() >= (prefix.size() + minParamLenth) &&
                str.compare(0, prefix.size(), prefix) == 0;
        }

    private:

        void AddCue(IMediaCue const& cue)
        {
            mutex.lock();
            try
            {
                if (Metadata::ApiInformation::IsApiContractPresent(L"Windows.Phone.PhoneContract", 1, 0))
                {
                    /*This is a fix only to work around a bug in windows phones: when 2 different cues have the exact same start position and length, the runtime panics and throws an exception
                    The problem has only been observed in external subtitles so far, and only on phones. Might also be present on ARM64 devices*/
                    bool individualCue = true;
                    if (this->timedMetadataKind == TimedMetadataKind::Subtitle)
                    {
                        for (int i = SubtitleTrack.Cues().Size() - 1; i >= 0; i--)
                        {
                            auto existingSub = SubtitleTrack.Cues().GetAt(i).as<TimedTextCue>();

                            if (existingSub.StartTime() == cue.StartTime() && existingSub.Duration() == cue.Duration())
                            {
                                individualCue = false;
                                auto timedTextCue = cue.as<TimedTextCue>();
                                for (auto l : timedTextCue.Lines())
                                {
                                    existingSub.Lines().Append(l);
                                }
                            }

                            break;
                        }
                    }
                    if (individualCue)
                    {
                        DispatchCueToTrack(cue);
                    }
                }
                else
                {
                    DispatchCueToTrack(cue);
                }
            }
            catch (...)
            {
                OutputDebugString(L"Failed to add subtitle cue.");
            }
            mutex.unlock();
        }

        void DispatchCueToTrack(IMediaCue const& cue)
        {
            if (m_config.as<implementation::MediaSourceConfig>()->IsExternalSubtitleParser)
            {
                SubtitleTrack.AddCue(cue);
            }
            else if (isPreviousCueInfiniteDuration)
            {
                pendingRefCues.push_back(ReferenceCue(cue));
                TriggerUpdateCues();
            }
            else
            {
                pendingCues.push_back(cue);
                TriggerUpdateCues();
            }
        }

        void OnRefCueEntered(TimedMetadataTrack const& sender, MediaCueEventArgs const& args)
        {
            UNREFERENCED_PARAMETER(sender);
            mutex.lock();
            try {
                //remove all cues from subtitle track
                while (SubtitleTrack.Cues().Size() > 0)
                {
                    SubtitleTrack.RemoveCue(SubtitleTrack.Cues().GetAt(0));
                }
                auto refCue = static_cast<ReferenceCue>(args.Cue());
                SubtitleTrack.AddCue(refCue);
                referenceTrack.RemoveCue(refCue);
            }
            catch (...)
            {
            }
            mutex.unlock();
        }

        void OnCueEntered(TimedMetadataTrack const& sender, MediaCueEventArgs const& args)
        {
            UNREFERENCED_PARAMETER(sender);
            mutex.lock();
            try
            {
                //cleanup old cues to free memory
                std::vector<IMediaCue> remove;
                for (auto cue : SubtitleTrack.Cues())
                {
                    if (cue.StartTime() + cue.Duration() < args.Cue().StartTime())
                    {
                        remove.push_back(cue);
                    }
                }

                for (auto &cue : remove)
                {
                    SubtitleTrack.RemoveCue(cue);
                }
            }
            catch (...)
            {
                OutputDebugString(L"Failed to cleanup old cues.");
            }
            mutex.unlock();
        }

        void TriggerUpdateCues()
        {
            if (dispatcher != nullptr && IsEnabled())
            {
                dispatcher.RunAsync(winrt::Windows::UI::Core::CoreDispatcherPriority::Normal,
                    weak_handler(this, &SubtitleProvider::StartDispatcherTimer));
            }
            else
            {
                OnTick(nullptr, nullptr);
            }
        }

        void StartDispatcherTimer()
        {
            if (timer == nullptr)
            {
                timer = winrt::Windows::UI::Xaml::DispatcherTimer();
                timer.Interval(TimeSpan(10000));
                timer.Tick(weak_handler(this, &SubtitleProvider::OnTick));
            }
            timer.Start();
        }

        void OnTick(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& args)
        {
            UNREFERENCED_PARAMETER(sender);
            UNREFERENCED_PARAMETER(args);
            mutex.lock();

            try
            {
                for (auto &cue : pendingChangedDurationCues)
                {
                    SubtitleTrack.RemoveCue(cue);
                    SubtitleTrack.AddCue(cue);
                }

                for (auto &cue : pendingCues)
                {
                    SubtitleTrack.AddCue(cue);
                }

                if (pendingRefCues.size() > 0)
                {
                    EnsureRefTrackInitialized();

                    for (auto &cue : pendingRefCues)
                    {
                        referenceTrack.AddCue(cue);
                    }
                }
            }
            catch (...)
            {
                OutputDebugString(L"Failed to add pending subtitle cues.");
            }

            pendingCues.clear();
            pendingRefCues.clear();
            pendingChangedDurationCues.clear();

            if (SubtitleDelay != newDelay)
            {
                try
                {
                    UpdateCuePositions();
                }
                catch (...)
                {
                }
                SubtitleDelay = newDelay;
            }

            if (timer != nullptr)
            {
                timer.Stop();
            }

            mutex.unlock();
        }

        void UpdateCuePositions()
        {
            auto track = SubtitleTrack;
            auto cues = to_vector<IMediaCue>(track.Cues());
            while (track.Cues().Size() > 0)
            {
                track.RemoveCue(track.Cues().GetAt(0));
            }

            std::vector<std::pair<IMediaCue, long long>> newNegativePositionCues;

            for (auto &c : cues)
            {
                TimeSpan cStartTime = c.StartTime();

                //check to see if this cue had negative duration
                if (c.StartTime().count() == 0)
                {
                    for (size_t i = 0; i < negativePositionCues.size(); i++)
                    {
                        auto element = negativePositionCues.at(i);
                        if (c == element.first)
                        {
                            cStartTime = TimeSpan(element.second);
                            break;
                        }
                    }
                }

                TimeSpan originalStartPosition = TimeSpan(cStartTime.count() - SubtitleDelay.count());
                TimeSpan newStartPosition = TimeSpan(originalStartPosition.count() + newDelay.count());
                //start time cannot be negative.
                if (newStartPosition.count() < 0)
                {
                    newNegativePositionCues.emplace_back(c, newStartPosition.count());
                    newStartPosition = TimeSpan(0);
                }

                c.StartTime(newStartPosition);
                track.AddCue(c);
            }

            negativePositionCues = newNegativePositionCues;
        }

        void EnsureRefTrackInitialized()
        {
            if (referenceTrack == nullptr)
            {
                referenceTrack = TimedMetadataTrack(L"ReferenceTrack_" + Name, L"", TimedMetadataKind::Custom);
                referenceTrack.CueEntered(weak_handler(this, &SubtitleProvider::OnRefCueEntered));
                PlaybackItem.TimedMetadataTracksChanged(weak_handler(this, &SubtitleProvider::OnTimedMetadataTracksChanged));
                PlaybackItem.Source().ExternalTimedMetadataTracks().Append(referenceTrack);
            }
        }

        void OnTimedMetadataTracksChanged(MediaPlaybackItem const& sender, IVectorChangedEventArgs const& args)
        {
            // enable ref track
            if (args.CollectionChange() == CollectionChange::ItemInserted &&
                sender.TimedMetadataTracks().GetAt(args.Index()) == referenceTrack)
            {
                PlaybackItem.TimedMetadataTracks().SetPresentationMode(
                    args.Index(), TimedMetadataTrackPresentationMode::Hidden);
            }
        }

        void OnTrackFailed(TimedMetadataTrack const& sender, TimedMetadataTrackFailedEventArgs const& args)
        {
            UNREFERENCED_PARAMETER(sender);
            UNREFERENCED_PARAMETER(args);
            OutputDebugString(L"Subtitle track error.");
        }

        void ClearSubtitles()
        {
            try
            {
                pendingCues.clear();
                pendingRefCues.clear();
                isPreviousCueInfiniteDuration = false;
                negativePositionCues.clear();

                if (referenceTrack != nullptr)
                {
                    while (referenceTrack.Cues().Size() > 0)
                    {
                        referenceTrack.RemoveCue(referenceTrack.Cues().GetAt(0));
                    }
                }

                while (SubtitleTrack.Cues().Size() > 0)
                {
                    SubtitleTrack.RemoveCue(SubtitleTrack.Cues().GetAt(0));
                }
            }
            catch (...)
            {
            }
        }

    public:

        void Flush() override
        {
            if (!m_config.as<implementation::MediaSourceConfig>()->IsExternalSubtitleParser)
            {
                CompressedSampleProvider::Flush();

                mutex.lock();

                if (dispatcher)
                {
                    dispatcher.RunAsync(winrt::Windows::UI::Core::CoreDispatcherPriority::Normal,
                        weak_handler(this, &SubtitleProvider::ClearSubtitles));
                }
                else
                {
                    ClearSubtitles();
                }

                mutex.unlock();
            }
        }

    private:

        std::recursive_mutex mutex;
        int cueCount = 0;
        std::vector<IMediaCue> pendingCues;
        std::vector<IMediaCue> pendingRefCues;
        std::vector<IMediaCue> pendingChangedDurationCues;
        TimedMetadataKind timedMetadataKind;
        winrt::Windows::UI::Core::CoreDispatcher dispatcher = { nullptr };
        winrt::Windows::UI::Xaml::DispatcherTimer timer = { nullptr };
        TimeSpan newDelay{};
        std::vector<std::pair<IMediaCue, long long>> negativePositionCues;
        bool isPreviousCueInfiniteDuration = false;
        IMediaCue infiniteDurationCue = { nullptr };
        IMediaCue lastExtendedDurationCue = { nullptr };
        TimedMetadataTrack referenceTrack = { nullptr };
        const long long InfiniteDuration = ((long long)0xFFFFFFFF) * 10000;

    };
}



