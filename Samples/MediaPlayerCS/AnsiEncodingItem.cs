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
        public CharacterEncoding FFTable
        {
            get;
            private set;
        }


        public AnsiEncodingItem(CharacterEncoding table)
        {
            this.FFTable = table;
        }

        public override string ToString()
        {
            return FFTable.Name + " | " + FFTable.WindowsCharacterEncoding;
        }
    }
}
