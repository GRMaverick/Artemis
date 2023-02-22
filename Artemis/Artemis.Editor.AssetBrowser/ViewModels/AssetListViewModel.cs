using Artemis.Editor.Settings;

namespace Artemis.Editor.AssetBrowser.ViewModels
{
    // All the code in this file is included in all platforms.
    public interface IAssetListViewModel
    { 
    }

    public class AssetItemViewModel
    {
        public string Name { get; set; } = string.Empty;
        public string ContentPath { get; set; } = string.Empty;
        public string AbsolutePath { get; set; } = string.Empty;
        //public AssetItemType Type { get; set;} = AssetItemType.Unknown;
        public AssetItemViewModel[] Children { get; set; } = null;
    }

    public class AssetListViewModel : IAssetListViewModel
    {
        public AssetItemViewModel _rootItem { get; private set; } = null;
        
        public AssetListViewModel(ISettingsService settingsService)
        {
            _settingsService = settingsService;

            _rootItem = new AssetItemViewModel()
            {
                Name = "Content",
                ContentPath = _settingsService.SourceAssetDirectory,
                AbsolutePath = string.Empty,
                //Type = AssetItemType.Folder,
            };

            //BuildAssetTree(_settingsService.SourceAssetDirectory);
        }

        private void BuildAssetTree(AssetItemViewModel _node)
        {
            string[] items = Directory.GetDirectories(_node.AbsolutePath);
            foreach(string item in items)
            {

            }
        }

        private readonly ISettingsService _settingsService;
    }
}