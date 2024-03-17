#pragma once
#include "pch.h"
#include <winrt/Windows.Media.Core.h>
#include "CompressedSampleProvider.h"
#include "NativeBufferFactory.h"
#include "ReferenceCue.h"
#include "AvCodecContextHelpers.h"
#include <winrt/FFmpegInteropX.h>


using namespace winrt::Windows::UI::Core;
using namespace winrt::Windows::Media::Playback;
using namespace winrt::Windows::Media::Core;
using namespace winrt::Windows::Foundation;

#ifdef Win32
using namespace winrt::Microsoft::UI::Dispatching;
#else
using namespace winrt::Windows::System;
#endif

class SubtitleProvider :
    public CompressedSampleProvider, public std::enable_shared_from_this<SubtitleProvider>
{
public:
    TimedMetadataTrack SubtitleTrack = { nullptr };

    winrt::weak_ref<MediaPlaybackItem> PlaybackItemWeak = { nullptr };

    SubtitleProvider(std::shared_ptr<FFmpegReader> reader,
        AVFormatContext* avFormatCtx,
        AVCodecContext* avCodecCtx,
        MediaSourceConfig const& config,
        int index,
        TimedMetadataKind const& ptimedMetadataKind,
        DispatcherQueue const& pdispatcher)
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

        if (!IsExternal())
        {
            SubtitleTrack.TrackFailed(weak_handler(this, &SubtitleProvider::OnTrackFailed));
        }

        InitializeStreamInfo();

        m_pAvStream->discard = AVDISCARD_DEFAULT;

        return S_OK;
    }

public:

    virtual void InitializeStreamInfo() override
    {
        auto forced = (m_pAvStream->disposition & AV_DISPOSITION_FORCED) == AV_DISPOSITION_FORCED;

        streamInfo = SubtitleStreamInfo(Name, Language, CodecName, (StreamDisposition)m_pAvStream->disposition,
            false, forced, SubtitleTrack, IsExternal(), m_streamIndex);
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
        try
        {
            TimeSpan position = ConvertPosition(packet->pts);
            TimeSpan duration = ConvertDuration(packet->duration);
            IMediaCue changedCue{ nullptr };

            auto cue = CreateCue(packet, &position, &duration);
            if (position.count() >= 0)
            {
                std::lock_guard lock(mutex);

                // apply subtitle delay
                position += streamDelay;
                if (position.count() < 0)
                {
                    if (cue)
                    {
                        negativePositionCues.emplace_back(cue, position.count());
                    }
                    position = std::chrono::seconds(0);
                }

                // clip previous extended duration cue, if it did not overlap originally but overlaps now, gap too small
                if (lastExtendedDurationCue)
                {
                    auto lastStartTime = lastExtendedDurationCue.StartTime();
                    auto lastDuration = lastExtendedDurationCue.Duration();
                    auto lastEndTime = lastStartTime + lastDuration;
                    bool isNewOverlap = m_config.Subtitles().PreventModifiedSubtitleDurationOverlap() &&
                        lastExtendedDurationCueOriginalEndTime <= position &&
                        lastEndTime > position;
                    bool isGapTooSmall = lastEndTime - position < m_config.Subtitles().ModifiedSubtitleDurationGap();
                    if (isNewOverlap || isGapTooSmall)
                    {
                        auto diff = position - (lastExtendedDurationCue.StartTime() + lastExtendedDurationCue.Duration() + m_config.Subtitles().ModifiedSubtitleDurationGap());
                        auto newDuration = lastExtendedDurationCue.Duration() + diff;
                        auto originalDuration = lastExtendedDurationCueOriginalEndTime - lastStartTime;
                        if (newDuration < originalDuration)
                        {
                            newDuration = originalDuration;
                        }

                        lastExtendedDurationCue.Duration(newDuration);
                        auto newEndTime = lastStartTime + newDuration;
                        if (IsPlayingLive() && newEndTime != position)
                        {
                            // add separate update point
                            AddUpdatePoint(lastExtendedDurationCue.StartTime() + newDuration, lastExtendedDurationCue);
                        }
                        else
                        {
                            // update along with added cue
                            changedCue = lastExtendedDurationCue;
                        }
                    }
                }

                lastExtendedDurationCue = nullptr;

                // extend duration of current cue if needed
                if (duration.count() < 0)
                {
                    duration = TimeSpan(InfiniteDuration);
                }
                else if (duration.count() < InfiniteDuration)
                {
                    if (m_config.Subtitles().AdditionalSubtitleDuration().count() != 0)
                    {
                        lastExtendedDurationCue = cue;
                        lastExtendedDurationCueOriginalEndTime = position + duration;
                        duration += m_config.Subtitles().AdditionalSubtitleDuration();
                    }
                    if (duration < m_config.Subtitles().MinimumSubtitleDuration())
                    {
                        if (lastExtendedDurationCue != cue)
                        {
                            lastExtendedDurationCue = cue;
                            lastExtendedDurationCueOriginalEndTime = position + duration;
                        }
                        duration = m_config.Subtitles().MinimumSubtitleDuration();
                    }
                }

                // fixup infinite duration cue
                if (infiniteDurationCue)
                {
                    auto newDuration = position - infiniteDurationCue.StartTime();
                    if (newDuration.count() < 0)
                    {
                        newDuration = TimeSpan{ 0 };
                    }
                    infiniteDurationCue.Duration(newDuration);
                    if (IsPlayingLive())
                    {
                        changedCue = infiniteDurationCue;
                    }
                    infiniteDurationCue = nullptr;
                }

                if (cue)
                {
                    cue.StartTime(position);
                    cue.Duration(duration);
                    AddCue(cue, changedCue);

                    if (duration.count() >= InfiniteDuration)
                    {
                        infiniteDurationCue = cue;
                    }
                }
                else if (changedCue)
                {
                    AddUpdatePoint(position, changedCue);
                }
            }
        }
        catch (...)
        {
            OutputDebugString(L"Failed to create subtitle cue.");
        }
        av_packet_free(&packet);
    }

    void SetStreamDelay(TimeSpan const& newDelay) override
    {
        SetSubtitleDelay(newDelay);
    }

private:

    void SetSubtitleDelay(TimeSpan const& delay)
    {
        std::lock_guard lock(mutex);

        if (streamDelay != delay)
        {
            streamDelay = delay;
            if (SubtitleTrack)
            {
                if (IsPlayingLive())
                {
                    AddUpdatePoint(TimeSpan{ 0 }, nullptr);
                }
                else
                {
                    UpdateCuePositions();
                    actualSubtitleDelay = delay;
                }
            }
        }
    }

    void AddUpdatePoint(TimeSpan startTime, IMediaCue const& changedCue)
    {
        std::lock_guard lock(mutex);
        try
        {
            if (IsPlayingLive())
            {
                EnsureRefTrackInitialized();
                IMediaCue refCue = winrt::make<ReferenceCue>(startTime, TimeSpan{ InfiniteDuration }, changedCue);
                referenceTrack.AddCue(refCue);
            }
        }
        catch (...)
        {
            OutputDebugString(L"Failed to add subtitle cue.");
        }
    }

    void AddCue(IMediaCue const& cue, IMediaCue const& changedCue)
    {
        std::lock_guard lock(mutex);
        try
        {
            if (!IsPlayingLive())
            {
                SubtitleTrack.AddCue(cue);
            }
            else
            {
                EnsureRefTrackInitialized();
                auto refCue = winrt::make<ReferenceCue>(cue, changedCue);
                referenceTrack.AddCue(refCue);
            }
        }
        catch (...)
        {
            OutputDebugString(L"Failed to add subtitle cue.");
        }
    }

    void OnRefCueEntered(TimedMetadataTrack const& sender, MediaCueEventArgs const& args)
    {
        UNREFERENCED_PARAMETER(sender);
        std::lock_guard lock(mutex);
        try {
            auto enteredCue = args.Cue();
            auto refCue = enteredCue.as<ReferenceCue>();

            referenceTrack.RemoveCue(enteredCue);

            auto changedCue = refCue->ChangedCue();
            if (changedCue)
            {
                SubtitleTrack.RemoveCue(changedCue);
                SubtitleTrack.AddCue(changedCue);
            }

            auto addedCue = refCue->AddedCue();
            if (addedCue)
            {
                SubtitleTrack.AddCue(addedCue);
            }

            if (actualSubtitleDelay != streamDelay)
            {
                try
                {
                    UpdateCuePositions();
                }
                catch (...)
                {
                }
                actualSubtitleDelay = streamDelay;
            }
        }
        catch (...)
        {
        }
    }

    void UpdateCuePositions()
    {
        auto& track = SubtitleTrack;
        auto cues = to_vector<IMediaCue>(track.Cues());

        std::vector<std::pair<IMediaCue, long long>> newNegativePositionCues;

        for (const auto& c : cues)
        {
            TimeSpan cStartTime = c.StartTime();

            //check to see if this cue had negative duration
            if (c.StartTime().count() == 0)
            {
                for (size_t i = 0; i < negativePositionCues.size(); i++)
                {
                    auto& element = negativePositionCues.at(i);
                    if (c == element.first)
                    {
                        cStartTime = TimeSpan(element.second);
                        break;
                    }
                }
            }

            TimeSpan originalStartPosition = TimeSpan(cStartTime.count() - streamDelay.count());
            TimeSpan newStartPosition = TimeSpan(originalStartPosition.count() + actualSubtitleDelay.count());
            //start time cannot be negative.
            if (newStartPosition.count() < 0)
            {
                newNegativePositionCues.emplace_back(c, newStartPosition.count());
                newStartPosition = TimeSpan(0);
            }

            c.StartTime(newStartPosition);
        }

        auto activeCues = to_vector<IMediaCue>(track.ActiveCues());
        for (const auto& c : activeCues)
        {
            track.RemoveCue(c);
            track.AddCue(c);
        }

        negativePositionCues = newNegativePositionCues;
    }

    void EnsureRefTrackInitialized()
    {
        auto playbackItem = PlaybackItemWeak.get();
        if (!playbackItem) return;
        if (referenceTrack == nullptr)
        {
            referenceTrack = TimedMetadataTrack(L"ReferenceTrack_" + Name, L"", TimedMetadataKind::Custom);
            referenceTrack.CueEntered(weak_handler(this, &SubtitleProvider::OnRefCueEntered));
            playbackItem.TimedMetadataTracksChanged(weak_handler(this, &SubtitleProvider::OnTimedMetadataTracksChanged));
            playbackItem.Source().ExternalTimedMetadataTracks().Append(referenceTrack);
        }
    }

    void OnTimedMetadataTracksChanged(MediaPlaybackItem const& sender, IVectorChangedEventArgs const& args)
    {
        // enable ref track
        auto playbackItem = PlaybackItemWeak.get();
        if (!playbackItem) return;
        if (args.CollectionChange() == CollectionChange::ItemInserted &&
            sender.TimedMetadataTracks().GetAt(args.Index()) == referenceTrack)
        {
            PlaybackItemWeak.get().TimedMetadataTracks().SetPresentationMode(
                args.Index(), TimedMetadataTrackPresentationMode::Hidden);
        }
    }

    void OnTrackFailed(TimedMetadataTrack const& sender, TimedMetadataTrackFailedEventArgs const& args)
    {
        UNREFERENCED_PARAMETER(sender);
        UNREFERENCED_PARAMETER(args);
        OutputDebugString(L"Subtitle track error.");
    }

public:

    void EnableStream() override
    {
        DebugMessage(L"EnableStream\n");
        m_isEnabled = true;
    }

    void DisableStream() override
    {
        DebugMessage(L"DisableStream\n");
        m_isEnabled = false;
    }

    bool inline IsExternal()
    {
        return m_config.as<implementation::MediaSourceConfig>()->IsExternalSubtitleParser;
    }

    bool inline IsPlayingLive()
    {
        return !IsExternal() && IsEnabled();
    }

    void ClearSubtitles()
    {
        std::lock_guard lock(mutex);
        try
        {
            infiniteDurationCue = nullptr;
            lastExtendedDurationCue = nullptr;
            negativePositionCues.clear();

            if (referenceTrack != nullptr)
            {
                auto size = referenceTrack.Cues().Size();
                while (size > 0)
                {
                    referenceTrack.RemoveCue(referenceTrack.Cues().GetAt(size - 1));
                    size = referenceTrack.Cues().Size();
                }
            }

            auto size = SubtitleTrack.Cues().Size();
            while (size > 0)
            {
                SubtitleTrack.RemoveCue(SubtitleTrack.Cues().GetAt(size - 1));
                size = SubtitleTrack.Cues().Size();
            }
        }
        catch (...)
        {
        }
    }

public:

    void Flush(bool flushBuffers) override
    {
        CompressedSampleProvider::Flush(flushBuffers);

        if (!IsExternal() && flushBuffers)
        {
            ClearSubtitles();
        }
    }

protected:
    std::recursive_mutex mutex;

private:

    int cueCount = 0;
    TimedMetadataKind timedMetadataKind;
    DispatcherQueue dispatcher = { nullptr };
    DispatcherQueueTimer timer = { nullptr };
    TimeSpan actualSubtitleDelay{};
    std::vector<std::pair<IMediaCue, long long>> negativePositionCues;
    IMediaCue infiniteDurationCue = { nullptr };
    IMediaCue lastExtendedDurationCue = { nullptr };
    TimeSpan lastExtendedDurationCueOriginalEndTime{};
    TimedMetadataTrack referenceTrack = { nullptr };
    const long long InfiniteDuration = ((long long)0xFFFFFFFF) * 10000;

};



