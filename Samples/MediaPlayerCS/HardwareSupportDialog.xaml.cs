using FFmpegInterop;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;
// The Blank Page item template is documented at https://go.microsoft.com/fwlink/?LinkId=234238

namespace MediaPlayerCS
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class HardwareSupportDialog : ContentDialog
    {
        public HardwareSupportDialog()
        {
            this.InitializeComponent();
            this.Loaded += HardwareSupportPage_Loaded;
        }

        private async void HardwareSupportPage_Loaded(object sender, RoutedEventArgs e)
        {
            await CodecChecker.InitializeAsync();
            await CodecChecker.RefreshAsync();
            List<CodecInformationModel> items = new List<CodecInformationModel>();

            items.Add(new CodecInformationModel("H264", CodecChecker.GetHardwareAccelerationH264View()));
            items.Add(new CodecInformationModel("HEVC", CodecChecker.GetHardwareAccelerationHEVCView()));
            items.Add(new CodecInformationModel("MPEG2", CodecChecker.GetHardwareAccelerationMPEG2View()));
            items.Add(new CodecInformationModel("VC1", CodecChecker.GetHardwareAccelerationVC1View()));
            items.Add(new CodecInformationModel("VP8", CodecChecker.GetHardwareAccelerationVP8View()));
            items.Add(new CodecInformationModel("VP9", CodecChecker.GetHardwareAccelerationVP9View()));
            items.Add(new CodecInformationModel("WMV3", CodecChecker.GetHardwareAccelerationWMV3View()));

            lsvHardwareInfo.ItemsSource = items;

        }
    }


    public class CodecInformationModel
    {
        public string CodecName { get; private set; }

        public string SupportStatus
        {
            get
            {
                if (ProfileInfos.Any())
                {
                    return "Supported";
                }
                else return "Not supported";
            }
        }

        public IEnumerable<ProfileInformationView> ProfileInfos
        {
            get;
            private set;
        }

        public CodecInformationModel(string codecName, IEnumerable<ProfileInformationView> profileInfos)
        {
            CodecName = codecName ?? throw new ArgumentNullException(nameof(codecName));
            ProfileInfos = profileInfos ?? throw new ArgumentNullException(nameof(profileInfos));
        }
    }
}
