using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Media;
using Windows.Media.Playback;
using Windows.Media.Core;
using Windows.Foundation;
using System.Threading;

namespace FFmpegInterop.Helpers
{
    /// <summary>
    /// A memory efficient adapter for enabling gapless playback using FFmpegInterop. It works by storing just 2 MediaPlaybackItems in memory
    /// and requesting new ones when necessary
    /// </summary>
    public sealed class MediaPlaybackListAdapter : IDisposable
    {
        /// <summary>
        /// The MediaPlaybackList wrapped in the adapter. Use this setter only to consume along with the MediaPlayer. 
        /// DO NOT call any methods on it
        /// </summary>
        public MediaPlaybackList PlaybackList
        {
            get;
            private set;
        } = new MediaPlaybackList();


        public MediaPlayer Player
        {
            get;
            private set;
        }

        AutoResetEvent nextItemSignal = new AutoResetEvent(false);
        int currentIndex = -1;

        IMediaPlaybackItemProvider playbackItemProvider;


        public MediaPlaybackListAdapter(IMediaPlaybackItemProvider provider, MediaPlaybackItem firstItemToPlay)
        {
            AllocResources(provider, firstItemToPlay);
        }


        public MediaPlaybackListAdapter(IMediaPlaybackItemProvider provider, MediaPlaybackItem firstItemToPlay, MediaPlaybackItem gaplessPlaybackItem)
        {
            AllocResources(provider, firstItemToPlay, gaplessPlaybackItem);
        }

        private void AllocResources(IMediaPlaybackItemProvider provider, params MediaPlaybackItem[] items)
        {
            PlaybackList.CurrentItemChanged += PlaybackList_CurrentItemChanged; ;
            playbackItemProvider = provider;
            foreach (var i in items)
            {
                PlaybackList.Items.Add(i);
            }
        }

        private async void PlaybackList_CurrentItemChanged(MediaPlaybackList sender, CurrentMediaPlaybackItemChangedEventArgs args)
        {
            currentIndex++;
            if (sender.Items.Count < 2)
            {
                await AddItemAtTheEndOfList();
            }
            else
            {
                //there was an item opened
                if (currentIndex == 1)
                {
                    RemoveFirstItem();
                    await AddItemAtTheEndOfList();
                    currentIndex = 0;
                }
            }

        }



        private void RemoveFirstItem()
        {
            PlaybackList.Items[0].Source.Dispose();
            PlaybackList.Items.RemoveAt(0);
        }



        /// <summary>
        /// occurs when the media playback list needs a new "next item". Send a null item to mark the end of the list.
        /// </summary>
        public event EventHandler<MediaPlaybackItemRequestOperation> PlaybackItemRequest;

        /// <summary>
        /// Stops playback of current item, disposes it, clears the MediaPlaybackList, adds firstItem to item and then starts playback
        /// </summary>
        /// <param name="firstItem"></param>
        public void Reset(IEnumerable<MediaPlaybackItem> items)
        {
            if (!items.Any()) throw new ArgumentException();
            ResetInternal(items);
        }



        private void ResetInternal(IEnumerable<MediaPlaybackItem> items)
        {
            //copy the old items temporarily
            var oldItems = PlaybackList.Items.ToList();
            //add the new items to the list
            PlaybackList.Items.Clear();
            foreach (var i in items)
            {
                PlaybackList.Items.Add(i);
            }
            oldItems.ForEach((x) => { x.Source.Dispose(); });
        }

        /// <summary>
        /// adds the item obtained from itemsInternal at the given index
        /// </summary>
        /// <param name="index"></param>
        /// <returns></returns>
        private async Task AddItemAtTheEndOfList()
        {
            var newItem = await FetchNextItem(MediaPlaybackItemRequestType.NextItem);
            if (newItem != null)
                PlaybackList.Items.Add(newItem);
            nextItemSignal.Set();
        }



        /// <summary>
        /// Skips to the next item in the list. If the item is not yet loaded, it will wait for it to load
        /// </summary>
        public IAsyncAction MoveToNextItem()
        {
            return Task.Run(async () =>
            {
                if (PlaybackList.Items.Count == 2)
                {
                    SafeMoveNext();
                    await AddItemAtTheEndOfList();
                }
                else
                {
                    await AddItemAtTheEndOfList();
                    SafeMoveNext();
                    //no next item
                    await AddItemAtTheEndOfList();
                }
            }).AsAsyncAction();
        }




        private void SafeMoveNext()
        {
            //move to the next item
            PlaybackList.MoveNext();
            RemoveFirstItem();
            //CurrentIndex now points to old item

        }

        /// <summary>
        /// Skips to the previous item, provided as param to this method. The current item will become the next item
        /// before call: currentitem (playing), nextItem  (gapless)
        /// after call: previousItem (playing), currentItem (gapless), nextItem (disposed)
        /// 
        /// This method is essentially Reset(MediaPlaybackItem, MediaPlaybackItem)
        /// </summary>
        public void SkipPrevious(MediaPlaybackItem previousItem)
        {

        }

        private async Task<MediaPlaybackItem> FetchNextItem(MediaPlaybackItemRequestType type)
        {
            return await playbackItemProvider.GetPlaybackItem();
        }

        public void Dispose()
        {

            //copy the old items temporarily
            var oldItems = PlaybackList.Items.ToList();

            foreach (var old in oldItems)
            {
                old.Source.Dispose();
            }

            oldItems.Clear();
            PlaybackList.Items.Clear();
            PlaybackList = null;

        }
    }
}
