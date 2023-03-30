using Artemis.Editor.AssetBrowser.ViewModels;

namespace Artemis.Editor.AssetBrowser.Views
{
    public partial class AssetListView : ContentView
    {
        public AssetListView()
        {
            InitializeComponent();
        }

        private void ViewCell_Tapped(object sender, EventArgs e)
        {

        }

        private void OnItemSelected(object sender, SelectedItemChangedEventArgs args)
        {
            if(BindingContext is IAssetListViewModel viewModel)
            {
                if (args.SelectedItem is AssetItemViewModel selected)
                {
                    if (selected.Type == Interfaces.AssetItemType.Folder)
                    {
                        viewModel.SelectedNode = selected.Name.Equals("..") ?
                            selected.Parent : selected;
                    }
                    else
                    {
                        // TODO: Open File Code.
                    }
                }
            }
        }
    }
}