using Artemis.Editor.Interfaces;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Artemis.Editor.AssetBrowser.ViewModels
{
    public class AssetItemViewModel : INotifyPropertyChanged
    {
        public string Name
        {
            get
            {
                return _name;
            }
            set
            {
                if (_name != value)
                {
                    _name = value;
                    OnPropertyChanged(nameof(Name));
                }
            }
        }
        public string ContentPath
        {
            get
            {
                return _contentPath;
            }
            set
            {
                if (_contentPath != value)
                {
                    _contentPath = value;
                    OnPropertyChanged(nameof(ContentPath));
                }
            }
        }
        public string AbsolutePath
        {
            get
            {
                return _absolutePath;
            }
            set
            {
                if (_absolutePath != value)
                {
                    _absolutePath = value;
                    OnPropertyChanged(nameof(AbsolutePath));
                }
            }
        }
        public AssetItemType Type
        {
            get
            {
                return _type;
            }
            set
            {
                if (_type != value)
                {
                    _type = value;
                    OnPropertyChanged(nameof(Type));
                }
            }
        }
        public AssetItemViewModel Parent
        {
            get
            {
                return _parent;
            }
            set
            {
                if(_parent != value)
                {
                    _parent = value;
                    OnPropertyChanged(nameof(Parent));
                }
            }
        }
        public ObservableCollection<AssetItemViewModel> Children
        {
            get
            {
                return _children;
            }
            set
            {
                if (_children != value)
                {
                    _children = value;
                    OnPropertyChanged(nameof(Children));
                }
            }
        }

        public event PropertyChangedEventHandler PropertyChanged;

        private string _name = string.Empty;
        private string _contentPath = string.Empty;
        private string _absolutePath = string.Empty;
        private AssetItemType _type = AssetItemType.Unknown;
        private AssetItemViewModel _parent = null;
        private ObservableCollection<AssetItemViewModel> _children = new();

        private void OnPropertyChanged(string name) =>
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));
    }
}
