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

        AutoResetEvent nextItemSignal = new AutoResetEvent(false);

        public int CurrentIndex
        {
            get;
            private set;
        }

        public int? NextIndex
        {
            get
            {
                if (CurrentIndex < itemsInternal.Count)
                    return CurrentIndex + 1;
                else return null;
            }
        }

        public int? PreviousIndex
        {
            get
            {
                if (CurrentIndex > 0)
                    return CurrentIndex - 1;
                else return null;
            }
        }

        List<IFfmpegInteropBuilder> itemsInternal = new List<IFfmpegInteropBuilder>();
        public IEnumerable<IFfmpegInteropBuilder> Items
        {
            get
            {
                return itemsInternal.AsEnumerable();
            }
        }

        public MediaPlaybackListAdapter(IEnumerable<IFfmpegInteropBuilder> items)
        {
            if (items == null) throw new ArgumentNullException("items");
            AllocResources(items);
        }

        private void AllocResources(IEnumerable<IFfmpegInteropBuilder> items)
        {
            itemsInternal.AddRange(items);
        }




        /// <summary>
        /// Stops playback of current item, disposes it, clears the MediaPlaybackList, adds firstItem to item and then starts playback
        /// </summary>
        /// <param name="firstItem"></param>
        public IAsyncAction Reset(IEnumerable<IFfmpegInteropBuilder> items)
        {
            if (!items.Any()) throw new ArgumentException();
            return ResetInternal(items).AsAsyncAction();
        }



        private async Task ResetInternal(IEnumerable<IFfmpegInteropBuilder> items)
        {
            //copy the old items temporarily
            var oldItems = PlaybackList.Items.ToList();
            //add the new items to the list
            PlaybackList.Items.Clear();
            CurrentIndex = 0;
            itemsInternal.ForEach((x) => x.Dispose());

            itemsInternal.Clear();
            itemsInternal.AddRange(items);

            oldItems.ForEach((x) => { x.Source.Dispose(); });

            var newItem = await FetchItemAtIndex(CurrentIndex);
            PlaybackList.Items.Add(newItem);

            if (NextIndex.HasValue)
            {
                //fetch the next item as well
                await AddItemAtTheEndOfList(NextIndex.Value);
            }
        }

        /// <summary>
        /// adds the item obtained from itemsInternal at the given index
        /// </summary>
        /// <param name="index"></param>
        /// <returns></returns>
        private async Task AddItemAtTheEndOfList(int index)
        {
            var newItem = await FetchItemAtIndex(index);
            PlaybackList.Items.Add(newItem);
            nextItemSignal.Set();

        }

        /// <summary>
        /// Skips to the next item in the list. If the item is not yet loaded, it will wait for it to load
        /// </summary>
        public IAsyncAction SkipNext()
        {
            return Task.Run(async () =>
            {
                if (PlaybackList.Items.Count == 2)
                {
                    await SafeMoveNext();
                }
                else
                {
                    //no next item
                    if (NextIndex.HasValue)
                    {
                        //there is one available, so fetch it. Should wait for pending operations, though
                        nextItemSignal.WaitOne();
                        if (PlaybackList.Items.Count == 2)
                        {
                            await SafeMoveNext();
                        }
                        else
                        {
                            //TODO: next item could not be fetched
                        }
                    }
                    else
                    {
                        //there is no next item. There are several possible cases here:
                        //The index is at the end of a 1item list, which is also the item being played.
                        //the index is at the end of a multiple items list, should we loop over or stop?

                        if (PlaybackList.AutoRepeatEnabled)
                        {
                            await AddItemAtTheEndOfList(0);
                        }
                        else
                        {
                            //see above
                        }
                    }
                }
            }).AsAsyncAction();
        }

        private async Task SafeMoveNext()
        {
            //move to the next item
            PlaybackList.MoveNext();
            PlaybackList.Items[0].Source.Dispose();
            PlaybackList.Items.RemoveAt(0);

            //CurrentIndex now points to old item
            itemsInternal[CurrentIndex].Reset();
            CurrentIndex += 1;
            if (NextIndex.HasValue)
            {
                await AddItemAtTheEndOfList(NextIndex.Value);
            }
            else
            {
                //we are now playing the last item in the list
                if (PlaybackList.AutoRepeatEnabled)
                {
                    await AddItemAtTheEndOfList(0);
                }
                else
                {
                    //see above
                }
            }
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

        private async Task<MediaPlaybackItem> FetchItemAtIndex(int startIndex)
        {
            if (startIndex >= itemsInternal.Count)
            {
                return null;
            }
            var builder = itemsInternal[startIndex];
            MediaPlaybackItem returnValue = await builder.OpenFFmpegAsync();

            return returnValue;
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
