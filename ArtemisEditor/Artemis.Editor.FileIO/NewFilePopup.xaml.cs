using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Artemis.Editor.Controls.ViewModels;
using Artemis.Editor.Interfaces;
using CommunityToolkit.Maui.Alerts;
using CommunityToolkit.Maui.Views;

namespace Artemis.Editor.FileIO
{
    public partial class NewFilePopup : Popup
    {
        private AssetItemType _assetItemType { get; set; } = AssetItemType.Unknown;
        public DestinationViewViewModel DestinationViewViewModel { get; set; } = new DestinationViewViewModel();

        public NewFilePopup()
        {
            InitializeComponent();
        }

        public NewFilePopup(AssetItemType assetItemType)
        {
            BindingContext = this;

            _assetItemType = assetItemType;

            DestinationViewViewModel = new DestinationViewViewModel(
                _assetItemType.ToString());

            InitializeComponent();
        }

        private async void SelectedClicked(object sender, EventArgs e)
        {
            try
            {
                var result = await CommunityToolkit.Maui.Storage.FolderPicker.Default.PickAsync(_token);
                if (result is not null)
                {
#if DEBUG
                    await Toast.Make($"Folder Picked: {result.Path}", CommunityToolkit.Maui.Core.ToastDuration.Short)
                        .Show(_token);
#endif
                    DestinationViewViewModel.SelectedLocation = result.Path;
                }
            }
            catch (Exception ex)
            {
                await Toast.Make($"Folder Picker Failed: {ex.Message}", CommunityToolkit.Maui.Core.ToastDuration.Short)
                    .Show(_token);
            }
        }

        private readonly CancellationToken _token;

        private void CreateClicked(object sender, EventArgs e)
        {
            if (FilenameEntry.Text != FilenameEntry.Placeholder && !string.IsNullOrEmpty(FilenameEntry.Text))
            {
                this.Handler.MauiContext.Services.GetService<IProjectSettings>()
                    .Create(DestinationViewViewModel.SelectedLocation, $"{FilenameEntry.Text}");

                Close();
            }
        }

        private void CancelClicked(object sender, EventArgs e)
        {
            Close();
        }
    }
}