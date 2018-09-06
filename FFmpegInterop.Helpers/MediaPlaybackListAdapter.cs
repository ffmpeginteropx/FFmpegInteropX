using System;
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;
using Windows.Foundation;
using Windows.Media.Playback;

namespace FFmpegInterop.Helpers
{
    public sealed class MediaPlaybackListAdapter : IDisposable
    {

        public IMediaPlaybackItemProvider PlaybackItemsProvider
        {
            get;
            private set;
        }

        public MediaPlayer CurrentPlayer
        {
            get;
            private set;
        }

        List<FFmpegInteropMSS> ActiveInteropMss = new List<FFmpegInteropMSS>();
        IAsyncOperation<FFmpegInteropMSS> currentFetchOperation = null;


        MediaPlaybackList PlaybackList;
        bool mediaEnded = false;
        bool sequenceEnded = false;
        AutoResetEvent _lock = new AutoResetEvent(false);
        AutoResetEvent _methodLock = new AutoResetEvent(true);

        public event EventHandler<CurrentMediaPlaybackItemChangedEventArgs> CurrentPlaybackItemChanged;

        public MediaPlaybackListAdapter(IMediaPlaybackItemProvider provider, MediaPlayer player)
        {
            AllocResources(provider, player, null);
        }

        public MediaPlaybackListAdapter(IMediaPlaybackItemProvider provider, MediaPlayer player, IEnumerable<MediaPlaybackItem> initialItems)
        {
            AllocResources(provider, player, initialItems);
        }

        public IAsyncAction Start()
        {
            return Task.Run(async () =>
            {
                _methodLock.WaitOne();
                try
                {
                    if (PlaybackList.Items.Count == 0)
                    {
                        var item = await PlaybackItemsProvider.GetNextItemAsync();
                        AddItemToPlaybackList(item);
                    }

                    CurrentPlayer.Source = PlaybackList;
                    _lock.Set();
                }
                finally
                {
                    _methodLock.Set();
                }
            }).AsAsyncAction();
        }

        /// <summary>
        /// moves to the next media playback item. Calling applications should handle the logic for fetching the next item.
        /// </summary>
        /// <returns>true if moving is allowed, false if the media playback items provider is empty</returns>
        public IAsyncOperation<bool> MoveNext()
        {
            return Task.Run(async () =>
            {
                _methodLock.WaitOne();

                if (sequenceEnded)
                {
                    return false;
                }

                try
                {
                    if (CurrentPlayer.Source != PlaybackList)
                    {
                        throw new InvalidOperationException("Call start first");
                    }
                    //if there is only 1 item in the list, or if we are at the end of the list, fetch the next one
                    if (PlaybackList.Items.Count == 1 || PlaybackList.CurrentItemIndex == PlaybackList.Items.Count - 1)
                    {
                        var item = await PlaybackItemsProvider.GetNextItemAsync();
                        AddItemToPlaybackList(item);
                    }

                    PlaybackList.MoveNext();
                    return true;
                }
                finally
                {
                    _methodLock.Set();
                }
            }).AsAsyncOperation<bool>();
        }

        public void SetVideoEffects(IReadOnlyList<AvEffectDefinition> videoEffects)
        {
            foreach (var interop in ActiveInteropMss)
            {
                interop.SetVideoEffects(videoEffects);
            }
        }

        public void SetAudioEffects(IReadOnlyList<AvEffectDefinition> audioEffects)
        {
            foreach (var interop in ActiveInteropMss)
            {
                interop.SetAudioEffects(audioEffects);
            }
        }

        public void RemoveAudioEffects()
        {
            foreach (var interop in ActiveInteropMss)
            {
                interop.DisableAudioEffects();
            }
        }

        public void RemoveVideoEffects()
        {
            foreach (var interop in ActiveInteropMss)
            {
                interop.DisableVideoEffects();
            }
        }

        private void AllocResources(IMediaPlaybackItemProvider provider, MediaPlayer player, IEnumerable<MediaPlaybackItem> initialItems)
        {
            this.PlaybackItemsProvider = provider ?? throw new ArgumentNullException("provider cannot be null");
            this.CurrentPlayer = player ?? throw new ArgumentNullException("player cannot be null");
            PlaybackList = new MediaPlaybackList();
            PlaybackList.CurrentItemChanged += PlaybackList_CurrentItemChanged;
            if (initialItems != null)
            {
                foreach (var item in initialItems)
                {
                    PlaybackList.Items.Add(item);
                }
            }

            CurrentPlayer.MediaOpened += CurrentPlayer_MediaOpened;
            CurrentPlayer.MediaEnded += CurrentPlayer_MediaEnded;
        }

        private void CurrentPlayer_MediaEnded(MediaPlayer sender, object args)
        {
            mediaEnded = true;
        }

        private void CurrentPlayer_MediaOpened(MediaPlayer sender, object args)
        {
            mediaEnded = false;
        }

        private async void PlaybackList_CurrentItemChanged(MediaPlaybackList sender, CurrentMediaPlaybackItemChangedEventArgs args)
        {
            ///forward the event on a background thread.
            Task.Run(() => { this.CurrentPlaybackItemChanged?.Invoke(sender, args); });

            //ask for the next item

            if (_lock.WaitOne(TimeSpan.FromMilliseconds(1)))
            {
                if (!sequenceEnded)
                {
                    var item = await PlaybackItemsProvider.GetNextItemAsync();

                    System.Diagnostics.Debug.WriteLine(sender.Items.Count);
                    AddItemToPlaybackList(item);
                    System.Diagnostics.Debug.WriteLine(sender.Items.Count);

                    if (mediaEnded)
                    {
                        sender.MoveTo((uint)sender.Items.Count - 1);
                        CurrentPlayer.Play();
                        mediaEnded = false;
                    }
                }
                _lock.Set();
            }
        }

        private void AddItemToPlaybackList(FFmpegInteropMSS item)
        {
            if (item != null)
            {
                if (item.PlaybackItem == null)
                {
                    throw new InvalidOperationException("ffmpeg interop object must have a properly initialized media playback item");
                }

                PlaybackList.Items.Add(item.PlaybackItem);
                ActiveInteropMss.Add(item);
            }
            else
            {
                sequenceEnded = true;
            }
        }

        public void Dispose()
        {
            CurrentPlayer.Source = null;
            PlaybackList.Items.Clear();
            foreach (var item in ActiveInteropMss)
            {
                item.PlaybackItem.Source.Dispose();
                item.Dispose();
            }
        }
    }
}
