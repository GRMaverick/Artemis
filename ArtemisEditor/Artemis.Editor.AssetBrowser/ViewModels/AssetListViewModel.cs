using Artemis.Editor.Interfaces;
using CommunityToolkit.Maui.Markup;
using Microsoft.Maui.Graphics;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.IO;
using System.Xml.Linq;

namespace Artemis.Editor.AssetBrowser.ViewModels
{
    // All the code in this file is included in all platforms.
    public interface IAssetListViewModel
    {
        AssetItemViewModel SelectedNode { get; set; }
    }

    public class AssetListViewModel : IAssetListViewModel, INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        public AssetItemViewModel SelectedNode
        {
            get
            {
                return _selectedNode;
            }
            set
            {
                if (_selectedNode != value)
                {
                    _selectedNode = value;
                    OnPropertyChanged(nameof(SelectedNode));
                }
            }
        }

        public AssetListViewModel(IProjectSettings projectSettings)
        {
            _projectSettings = projectSettings;
            _projectSettings.OnProjectLoad += OnProjectLoad;

            InitFileSystemWatcher();
            RebuildAssetTree();
        }

        private FileSystemWatcher _watcher = null;
        private AssetItemViewModel _rootNode = null;
        private AssetItemViewModel _selectedNode = null;
        private IProjectSettings _projectSettings = null;
        private readonly List<AssetItemType> _excludeFromView = new()
        {
            AssetItemType.Project,
        };

        private void OnPropertyChanged(string name) =>
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));

        private void RebuildAssetTree()
        {
            _rootNode = new AssetItemViewModel()
            {
                Name = "Root",
                ContentPath = "\\",
                AbsolutePath = _projectSettings.AbsoluteProjectDirectory,
                Type = AssetItemType.Folder,
            };

            BuildAssetTreeRecursive(_rootNode);

            SelectedNode = _rootNode;
        }

        private void BuildAssetTreeRecursive(AssetItemViewModel node)
        {
            if (string.IsNullOrEmpty(node.AbsolutePath) || node.Name.Equals(".."))
            {
                return;
            }

            string[] folders = Directory.GetDirectories(node.AbsolutePath);
            string[] files = Directory.GetFiles(node.AbsolutePath);

            for (int i = 0; i < folders.Length; i++)
            {
                node.Children.Add(new()
                {
                    Name = Path.GetFileNameWithoutExtension(folders[i]),
                    ContentPath = Path.GetRelativePath(_rootNode.AbsolutePath, folders[i]),
                    AbsolutePath = folders[i],
                    Type = AssetItemType.Folder,
                    Parent = node
                });

                BuildAssetTreeRecursive(node.Children[i]);
            }

            AddReturnDirectory(node);

            for (int i = 0; i < files.Length; i++)
            {
                _ = Enum.TryParse(Path.GetExtension(files[i]).Replace(".art", ""), out AssetItemType assetItemType);

                if (_excludeFromView.Contains(assetItemType))
                {
                    continue;
                }

                node.Children.Add(new()
                {
                    Name = Path.GetFileNameWithoutExtension(files[i]),
                    ContentPath = Path.GetRelativePath(_rootNode.AbsolutePath, files[i]),
                    AbsolutePath = files[i],
                    Type = assetItemType
                });

                _ = node.Children.Append(node.Children[i + folders.Length]);
            }

            SortNodeChildren(node);
        }

        private void AddReturnDirectory(AssetItemViewModel node)
        {
            if (node.Type == AssetItemType.Folder && node != _rootNode)
            {
                MainThread.BeginInvokeOnMainThread(() =>
                {
                    node.Children.Add(new()
                    {
                        Name = "..",
                        ContentPath = node.ContentPath,
                        AbsolutePath = node.AbsolutePath,
                        Type = AssetItemType.Folder,
                        Parent = node.Parent
                    });
                });
            }
        }

        private AssetItemViewModel FindParent(string path, AssetItemViewModel node = null)
        {
            foreach (var item in node.Children)
            {
                string x = Directory.GetParent(path).FullName;
                if (item.AbsolutePath.Equals(x))
                {
                    return item;
                }
                else
                {
                    return FindParent(path, item);
                }
            }

            return null;
        }

        private bool FindNode(string path, AssetItemViewModel node, out AssetItemViewModel foundNode)
        {
            foundNode = null;
            if(node.AbsolutePath.Equals(path))
            {
                foundNode = node;
                return true;
            }

            foreach (var item in node.Children)
            {
                if (item.AbsolutePath.Equals(path))
                {
                    foundNode = item;
                    break;
                }
                else
                {
                    if (node.Children.Count > 0)
                    {
                        if (FindNode(path, item, out foundNode))
                        {
                            break;
                        }
                    }
                }
            }

            return foundNode != null;
        }

        private void OnFSObjectCreated(object sender, FileSystemEventArgs e)
        {
            Console.WriteLine($"Created: {e.FullPath}");

            if (FindNode(Directory.GetParent(e.FullPath).FullName, _rootNode, out AssetItemViewModel parent))
            {
                AssetItemViewModel newVM = new()
                {
                    Name = Path.GetFileNameWithoutExtension(e.FullPath),
                    ContentPath = Path.GetRelativePath(_rootNode.AbsolutePath, e.FullPath),
                    AbsolutePath = e.FullPath,
                    Parent = parent
                };

                if (!Path.HasExtension(e.FullPath))
                {
                    newVM.Type = AssetItemType.Folder;

                    AddReturnDirectory(newVM);
                }
                else
                {
                    _ = Enum.TryParse(Path.GetExtension(e.FullPath).Replace(".art", ""), out AssetItemType assetItemType);

                    if (_excludeFromView.Contains(assetItemType))
                    {
                        return;
                    }
                    newVM.Type = assetItemType;
                }

                MainThread.BeginInvokeOnMainThread(() =>
                {
                    parent.Children.Add(newVM);

                    SortNodeChildren(parent);
                });
            }
        }

        private void OnFSObjectDeleted(object sender, FileSystemEventArgs e)
        {
            string fullPath = e.FullPath;
            Console.WriteLine($"Deleted: {fullPath}");

            if (FindNode(Directory.GetParent(fullPath).FullName, _rootNode, out AssetItemViewModel parent))
            {
                MainThread.BeginInvokeOnMainThread(() =>
                {
                    if (FindNode(fullPath, parent, out AssetItemViewModel target))
                    {
                        parent.Children.Remove(target);
                    }
                });
            }
        }

        private void OnFSObjectRenamed(object sender, RenamedEventArgs e)
        {
            Console.WriteLine($"Renamed: {e.FullPath}");

            if (FindNode(e.OldFullPath, _rootNode, out AssetItemViewModel target))
            {
                MainThread.BeginInvokeOnMainThread(() =>
                {
                    target.Name = Path.GetFileNameWithoutExtension(e.FullPath);
                    target.ContentPath = Path.GetRelativePath(_rootNode.AbsolutePath, e.FullPath);
                    target.AbsolutePath = e.FullPath;

                    SortNodeChildren(target.Parent);
                });
            }
        }

        private void OnProjectLoad(IProjectSettings projectSettings)
        {
            _projectSettings = projectSettings;

            InitFileSystemWatcher();
            RebuildAssetTree();
        }

        private void InitFileSystemWatcher()
        {
            if (!string.IsNullOrEmpty(_projectSettings.AbsoluteProjectDirectory))
            {
                _watcher = new FileSystemWatcher(_projectSettings.AbsoluteProjectDirectory)
                {
                    NotifyFilter = NotifyFilters.CreationTime
                        | NotifyFilters.DirectoryName
                        | NotifyFilters.FileName,
                    EnableRaisingEvents = true,
                    IncludeSubdirectories = true
                };
                _watcher.Created += OnFSObjectCreated;
                _watcher.Deleted += OnFSObjectDeleted;
                _watcher.Renamed += OnFSObjectRenamed;
            }
        }

        private static void SortNodeChildren(AssetItemViewModel node)
        {
            List<AssetItemViewModel> folders = node.Children.Where(n => n.Type == AssetItemType.Folder)
                .ToList()
                .OrderBy(n => n.Name)
                .ToList();
            List<AssetItemViewModel> files = node.Children.Where(n => n.Type != AssetItemType.Folder)
                .ToList()
                .OrderBy(n => n.Name)
                .ToList();

            List<AssetItemViewModel> newList = new();
            newList.AddRange(folders);
            newList.AddRange(files);

            node.Children = new ObservableCollection<AssetItemViewModel>(newList);
        }
    }
}