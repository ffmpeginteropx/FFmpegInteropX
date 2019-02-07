#pragma once

#include <string>
#include <codecvt>

#include "CompressedSampleProvider.h"
#include "StreamInfo.h"
#include "NativeBufferFactory.h"
#include <ReferenceCue.h>


namespace FFmpegInterop
{
	ref class SubtitleProvider abstract : CompressedSampleProvider
	{
		TimedMetadataTrack^ referenceTrack;


	internal:
		SubtitleProvider(FFmpegReader^ reader,
			AVFormatContext* avFormatCtx,
			AVCodecContext* avCodecCtx,
			FFmpegInteropConfig^ config,
			int index,
			TimedMetadataKind timedMetadataKind)
			: CompressedSampleProvider(reader, avFormatCtx, avCodecCtx, config, index)
		{
			this->timedMetadataKind = timedMetadataKind;
		}
		property TimedMetadataTrack^ SubtitleTrack;


		virtual IVectorView<TimedMetadataTrack^>^ GetMetadataTracks()
		{
			Vector<TimedMetadataTrack^>^ tracks = ref new Vector<TimedMetadataTrack^>();
			tracks->Append(SubtitleTrack);
			tracks->Append(referenceTrack);
			return tracks->GetView();
		}


		virtual HRESULT Initialize() override
		{
			InitializeNameLanguageCodec();
			referenceTrack = ref new TimedMetadataTrack("subtitleReferenceTrack", "", TimedMetadataKind::Custom);

			SubtitleTrack = ref new TimedMetadataTrack(Name, Language, timedMetadataKind);
			SubtitleTrack->Label = Name != nullptr ? Name : Language;
			if (!m_config->IsExternalSubtitleParser) {
				cueEnteredToken = referenceTrack->CueEntered += ref new Windows::Foundation::TypedEventHandler<Windows::Media::Core::TimedMetadataTrack ^, Windows::Media::Core::MediaCueEventArgs ^>(this, &FFmpegInterop::SubtitleProvider::OnRefCueEntered);
				cueExitedToken = referenceTrack->CueExited += ref new Windows::Foundation::TypedEventHandler<Windows::Media::Core::TimedMetadataTrack ^, Windows::Media::Core::MediaCueEventArgs ^>(this, &FFmpegInterop::SubtitleProvider::OnCueExited);
				trackFailedToken = referenceTrack->TrackFailed += ref new Windows::Foundation::TypedEventHandler<Windows::Media::Core::TimedMetadataTrack ^, Windows::Media::Core::TimedMetadataTrackFailedEventArgs ^>(this, &FFmpegInterop::SubtitleProvider::OnTrackFailed);
			}
			return S_OK;
		}

		virtual void NotifyVideoFrameSize(int width, int height, double aspectRatio)
		{
		}

		virtual IMediaCue^ CreateCue(AVPacket* packet, TimeSpan* position, TimeSpan *duration) = 0;

		virtual void QueuePacket(AVPacket *packet) override
		{
			if (packet->pos > maxCuePosition)
			{
				maxCuePosition = packet->pos;
			}
			else if (addedCues.find(packet->pos) != addedCues.end() && !m_config->IsExternalSubtitleParser)
			{
				av_packet_free(&packet);
				return;
			}

			TimeSpan position;
			TimeSpan duration;
			bool isDurationFixed = false;

			position.Duration = LONGLONG(av_q2d(m_pAvStream->time_base) * 10000000 * packet->pts) - m_startOffset;
			//LRC and SAMI subtitles last line reports -1 duration, so set it to the end of stream
			if (packet->duration < 0 && m_config->IsExternalSubtitleParser)
			{
				if (m_config->StreamTimeDuration.Duration > position.Duration)
				{
					duration.Duration = m_config->StreamTimeDuration.Duration - position.Duration;
				}
				else
				{
					//duration can't be computed, use default duration
					duration.Duration = m_config->DefaultTimedMetadataCueDuration.Duration;
				}
				isDurationFixed = true;
			}
			else
			{
				duration.Duration = LONGLONG(av_q2d(m_pAvStream->time_base) * 10000000 * packet->duration);
			}
			auto cue = CreateCue(packet, &position, &duration);
			if (cue && position.Duration >= 0 && duration.Duration > 0)
			{
				addedCues[packet->pos] = packet->pos;

				cue->StartTime = position;
				cue->Duration = duration;
				AddCue(cue);

				// only last cue should be fixed to stream time. inbetween fixed cues should be default (3 seconds) duration max.
				if (durationFixedCue)
				{
					TimeSpan maxDuration;
					maxDuration.Duration = cue->StartTime.Duration - durationFixedCue->StartTime.Duration;
					if (maxDuration.Duration <= 0 || maxDuration.Duration > m_config->DefaultTimedMetadataCueDuration.Duration)
					{
						maxDuration = m_config->DefaultTimedMetadataCueDuration;
					}

					durationFixedCue->Duration = m_config->DefaultTimedMetadataCueDuration;
					durationFixedCue = nullptr;
				}

				// remember this cue if we fixed it's duration
				if (isDurationFixed)
				{
					durationFixedCue = cue;
				}
			}

			av_packet_free(&packet);
		}

		// convert UTF-8 string to wstring
		std::wstring utf8_to_wstring(const std::string& str)
		{
			std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
			return myconv.from_bytes(str);
		}

		Platform::String ^ convertFromString(const std::wstring & input)
		{
			return ref new Platform::String(input.c_str(), (unsigned int)input.length());
		}

	private:

		void AddCue(IMediaCue^ cue)
		{
			mutex.lock();
			try
			{
				// to avoid flicker, we try to add new cues only after active cues are finished
				if (!m_config->IsExternalSubtitleParser && m_config->UseAntiFlickerForSubtitles && referenceTrack->ActiveCues->Size > 0)
				{
					bool addToPending = true;
					for each (auto active in referenceTrack->ActiveCues)
					{
						if (active->StartTime.Duration + active->Duration.Duration > cue->StartTime.Duration)
						{
							addToPending = false;
							break;
						}
					}
					if (addToPending)
					{
						pendingCues.push_back(ref new ReferenceCue(cue));
					}
					else
					{
						referenceTrack->AddCue(ref new ReferenceCue(cue));
					}
				}
				else if (Windows::Foundation::Metadata::ApiInformation::IsApiContractPresent("Windows.Phone.PhoneContract", 1, 0))
				{
					/*This is a fix only to work around a bug in windows phones: when 2 different cues have the exact same start position and length, the runtime panics and throws an exception
					The problem has only been observed in external subtitles so far, and only on phones. Might also be present on ARM64 devices*/
					bool individualCue = true;
					if (this->timedMetadataKind == TimedMetadataKind::Subtitle) {
						for (int i = SubtitleTrack->Cues->Size - 1; i >= 0; i--)
						{
							auto existingSub = (TimedTextCue^)SubtitleTrack->Cues->GetAt(i);

							if (existingSub->StartTime.Duration == cue->StartTime.Duration && existingSub->Duration.Duration == cue->Duration.Duration)
							{
								individualCue = false;
								auto timedTextCue = (TimedTextCue^)cue;
								for each(auto l in timedTextCue->Lines)
								{
									existingSub->Lines->Append(l);
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

		void DispatchCueToTrack(IMediaCue^ cue)
		{
			if (!m_config->IsExternalSubtitleParser)
			{
				referenceTrack->AddCue(ref new ReferenceCue(cue));
			}
			else
			{
				SubtitleTrack->AddCue(cue);
			}
		}

		void OnRefCueEntered(TimedMetadataTrack ^sender, MediaCueEventArgs ^args)
		{
			mutex.lock();
			try
			{
				//remove all cues from subtitle track
				while (SubtitleTrack->Cues->Size > 0)
				{
					auto c = SubtitleTrack->Cues->GetAt(0);
					SubtitleTrack->RemoveCue(c);
				}

				auto refCue = (ReferenceCue^)args->Cue;
				SubtitleTrack->AddCue(refCue->CueRef);
			}
			catch (...)
			{
			}
			mutex.unlock();
		}

		void OnCueExited(TimedMetadataTrack ^sender, MediaCueEventArgs ^args)
		{
			mutex.lock();
			try
			{
				for each (auto cue in pendingCues)
				{
					referenceTrack->AddCue(cue);
				}

				//SubtitleTrack->RemoveCue(args->Cue); // todo re-enable
			}
			catch (...)
			{
				OutputDebugString(L"Failed to add subtitle cue.");
			}
			pendingCues.clear();
			mutex.unlock();
		}

		void OnTrackFailed(TimedMetadataTrack ^sender, TimedMetadataTrackFailedEventArgs ^args)
		{
			OutputDebugString(L"Subtitle track error.");
		}

		std::mutex mutex;
		std::vector<IMediaCue^> pendingCues;
		std::map<int64, int64> addedCues;
		int64 maxCuePosition;
		EventRegistrationToken cueExitedToken;
		EventRegistrationToken trackFailedToken;
		EventRegistrationToken cueEnteredToken;
		TimedMetadataKind timedMetadataKind;
		IMediaCue^ durationFixedCue;

	public:
		virtual ~SubtitleProvider()
		{
			if (SubtitleTrack)
			{
				SubtitleTrack = nullptr;
			}
			if (referenceTrack)
			{
				referenceTrack->CueEntered -= cueEnteredToken;
				referenceTrack->CueExited -= cueExitedToken;
				referenceTrack->TrackFailed -= trackFailedToken;
				referenceTrack = nullptr;
			}
		}
	};

}




