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
		TimeSpan oldOffset, newOffset;
	internal:

		property CoreDispatcher^ Dispatcher;
		property SubtitleStreamInfo^ ExternalStream
		{
			SubtitleStreamInfo^ get()
			{
				return externalStream;
			}
		}

		ExternalSubtitleProvider(CoreDispatcher^ p_dispatcher, SubtitleStreamInfo^ p_externalStream)
		{
			this->Dispatcher = p_dispatcher;
			this->externalStream = p_externalStream;
		}

		void SetSubtitleOffset(TimeSpan newOffset, TimeSpan currentOffset)
		{
			mutex.lock();
			this->newOffset = newOffset;
			this->oldOffset = currentOffset;
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

				for each(auto c in cues)
				{
					TimeSpan originalStartPosition = { c->StartTime.Duration - oldOffset.Duration };
					TimeSpan newStartPosition = { originalStartPosition.Duration + newOffset.Duration };
					//start time cannot be negative.
					if (newStartPosition.Duration < 0)
					{
						newStartPosition.Duration = 0;
					}
					c->StartTime = newStartPosition;
					track->AddCue(c);
				}
			}
			catch (...)
			{

			}
			mutex.unlock();
		}

	};
}


