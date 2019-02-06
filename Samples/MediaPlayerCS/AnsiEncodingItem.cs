using FFmpegInterop;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MediaPlayerCS
{
    public class AnsiEncodingItem
    {
        public EncodingTable FFTable
        {
            get;
            private set;
        }


        public AnsiEncodingItem(EncodingTable table)
        {
            this.FFTable = table;
        }

        public override string ToString()
        {
            return FFTable.Name + " | " + FFTable.WindowsEncodingTable;
        }
    }
}
