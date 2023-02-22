using Artemis.Editor.AssetBrowser.ViewModels;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Artemis.Editor.ViewModels 
{ 
    public class MainPageViewModel
    {
        public IAssetListViewModel AssetItemViewModel { get; set; } = null;

        public MainPageViewModel(IAssetListViewModel assetItemViewModel) {
            this.AssetItemViewModel = assetItemViewModel;
        }
    }
}
