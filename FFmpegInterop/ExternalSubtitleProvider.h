#pragma once
#include <pch.h>
#include "StreamInfo.h"
#include <mutex>
#include <FFmpegInteropConfig.h>
#include <collection.h>

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Core;

using namespace std;

namespace FFmpegInterop {
	ref class ExternalSubtitleProvider
	{

		DispatcherTimer^ timer;
		SubtitleStreamInfo^ externalStream;

		recursive_mutex mutex;
		TimeSpan currentDelay, newDelay;
		vector<pair<IMediaCue^, long long>> negativePositionCues;
	internal:

		property CoreDispatcher^ Dispatcher;
		property SubtitleStreamInfo^ ExternalStream
		{
			SubtitleStreamInfo^ get()
			{
				return externalStream;
			}
		}

		ExternalSubtitleProvider(CoreDispatcher^ p_dispatcher, SubtitleStreamInfo^ p_externalStream, TimeSpan p_delay)
		{
			this->Dispatcher = p_dispatcher;
			this->externalStream = p_externalStream;
			this->currentDelay = p_delay;
		}

		void SetSubtitleDelay(TimeSpan delay)
		{
			mutex.lock();
			newDelay = delay;
			try {
				if (Dispatcher != nullptr) {
					Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal,
						ref new Windows::UI::Core::DispatchedHandler([this]
					{
						if (timer == nullptr)
						{
							timer = ref new Windows::UI::Xaml::DispatcherTimer();
							timer->Interval = ToTimeSpan(10000);
							timer->Tick += ref new Windows::Foundation::EventHandler<Platform::Object ^>(this, &FFmpegInterop::ExternalSubtitleProvider::OnTick);
						}
						timer->Start();
					}));
				}
				else
				{
					OnTick(nullptr, nullptr);
				}
			}
			catch (...)
			{

			}
			mutex.unlock();
		}

		void OnTick(Platform::Object ^ sender, Platform::Object ^ args)
		{
			mutex.lock();
			timer->Stop();

			try
			{
				auto track = ExternalStream->SubtitleTrack;
				auto cues = to_vector(track->Cues);
				while (track->Cues->Size > 0)
				{
					track->RemoveCue(track->Cues->GetAt(0));
				}

				vector<pair<IMediaCue^, long long>> newNegativePositionCues;

				for each(auto c in cues)
				{
					TimeSpan cStartTime = c->StartTime;

					//check to see if this cue had negative duration
					if (c->StartTime.Duration == 0)
					{
						int lookupIndex = -1;
						for (int i = 0; i < negativePositionCues.size(); i++)
						{
							auto element = negativePositionCues.at(i);
							if (c == element.first)
							{
								cStartTime.Duration = element.second;
								lookupIndex = i;
								break;
							}
						}
					}

					TimeSpan originalStartPosition = { cStartTime.Duration - currentDelay.Duration };
					TimeSpan newStartPosition = { originalStartPosition.Duration + newDelay.Duration };
					//start time cannot be negative.
					if (newStartPosition.Duration < 0)
					{
						newNegativePositionCues.emplace_back(c, newStartPosition.Duration);
						newStartPosition.Duration = 0;
					}

					c->StartTime = newStartPosition;
					track->AddCue(c);
				}
				negativePositionCues.erase(negativePositionCues.begin(), negativePositionCues.end());

				negativePositionCues = newNegativePositionCues;
			}
			catch (...)
			{

			}
			currentDelay = newDelay;
			mutex.unlock();
		}
	public:
		virtual ~ExternalSubtitleProvider()
		{
			negativePositionCues.erase(negativePositionCues.begin(), negativePositionCues.end());
		}

	};
}



