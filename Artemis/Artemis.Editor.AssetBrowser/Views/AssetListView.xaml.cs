using Artemis.Editor.AssetBrowser.ViewModels;

namespace Artemis.Editor.AssetBrowser.Views
{
    public partial class AssetListView : ContentView
    {
        private readonly IAssetListViewModel _viewModel;

        public AssetListView()
        {
            InitializeComponent();
        }

        public AssetListView(IAssetListViewModel viewModel)
        {
            InitializeComponent();

            _viewModel = viewModel;
        }
    }
}