using Artemis.Editor.AssetBrowser.ViewModels;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Artemis.Editor.ViewModels 
{ 
    public class EditorMainPageViewModel
    {
        public IAssetListViewModel AssetItemViewModel { get; set; } = null;

        public EditorMainPageViewModel(IAssetListViewModel assetItemViewModel) {
            this.AssetItemViewModel = assetItemViewModel;
        }
    }
}
