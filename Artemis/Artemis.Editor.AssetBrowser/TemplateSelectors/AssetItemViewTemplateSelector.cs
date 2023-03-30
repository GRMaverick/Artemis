using Artemis.Editor.AssetBrowser.ViewModels;
using Artemis.Editor.Interfaces;

namespace Artemis.Editor.AssetBrowser.TemplateSelectors
{
    public class AssetItemViewTemplateSelector : DataTemplateSelector
    {
        public DataTemplate FolderTemplate 
        { 
            get => _items.ContainsKey(AssetItemType.Folder) ? _items[AssetItemType.Folder] : null;
            set
            {
                _items[AssetItemType.Folder] = value;
            }
        }
        public DataTemplate ShaderTemplate
        {
            get => _items.ContainsKey(AssetItemType.Shader) ? _items[AssetItemType.Shader] : null;
            set
            {
                _items[AssetItemType.Shader] = value;
            }
        }
        public DataTemplate UnknownTemplate
        {
            get => _items.ContainsKey(AssetItemType.Unknown) ? _items[AssetItemType.Unknown] : null;
            set
            {
                _items[AssetItemType.Unknown] = value;
            }
        }
                
        protected override DataTemplate OnSelectTemplate(object item, BindableObject container)
        {
            if (item is AssetItemViewModel aivm && _items.ContainsKey(aivm.Type))
            {
                return _items[aivm.Type];
            }

            return _items[AssetItemType.Unknown];
        }

        private readonly Dictionary<AssetItemType, DataTemplate> _items = new();
    }
}
