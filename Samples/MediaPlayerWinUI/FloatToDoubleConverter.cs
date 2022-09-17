using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.UI.Xaml.Data;

namespace MediaPlayerWinUI
{
    public class FloatToDoubleConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, string language)
        {
            return (double)(float)value;
        }

        public object ConvertBack(object value, Type targetType, object parameter, string language)
        {
            return (float)(double)value;
        }
    }
}
