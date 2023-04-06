using Artemis.Editor.AssetBrowser.ViewModels;
using Artemis.Editor.Interfaces;
using Artemis.Editor.ViewModels;

using CommunityToolkit.Maui;
using CommunityToolkit.Maui.Alerts;
using CommunityToolkit.Maui.Views;
using System.Windows.Input;

namespace Artemis.Editor.Views;

public partial class EditorMainPage : ContentPage
{
    public ICommand NewAssetCommand { get; private set; } = null;
	public ICommand OpenAssetCommand { get; private set; } = null;

	public static IProjectSettings ProjectSettings { get; private set; } = null;
	public EditorMainPageViewModel ViewModel { get; private set; } = null;

	public EditorMainPage(EditorMainPageViewModel mainPageViewModel,
		IProjectSettings projectSettings)
	{
		BindingContext = this;

        ViewModel = mainPageViewModel;
		ProjectSettings = projectSettings;

        NewAssetCommand = new Command((param) => CreateNewFile(param));
		OpenAssetCommand = new Command((param) => OpenFile(param));

        InitializeComponent();
    }

	private void CreateNewFile(object param)
	{
        _ = Enum.TryParse(param.ToString(), out AssetItemType assetItemType);
		this.ShowPopup(new FileIO.NewFilePopup(assetItemType));
	}

	private async void OpenFile(object param)
	{
		try
		{
			var result = await FilePicker.PickAsync(new()
			{
				PickerTitle = "Open Asset",
			});

			if(result is not null)
			{
#if DEBUG
				await Toast.Make($"File Picked: {result.FileName}", CommunityToolkit.Maui.Core.ToastDuration.Short)
					.Show(_filePickerToken);
#endif

				_fileLoadActions[DetermineAssetItemType(Path.GetExtension(result.FileName))]?.Invoke(result.FullPath);
            }
		}
		catch(Exception ex)
        {
            await Toast.Make($"File Picker Failed: {ex.Message}", CommunityToolkit.Maui.Core.ToastDuration.Short)
                .Show(_filePickerToken);
        }
    }

	private static AssetItemType DetermineAssetItemType(string extension)
	{
        _ = Enum.TryParse(extension.Replace(".art", ""), out AssetItemType result);
        return result;
	}

	private readonly Dictionary<AssetItemType, Func<string, bool>> _fileLoadActions = new()
	{
		{ AssetItemType.Project, (fullPath) => ProjectSettings.Load(fullPath) },
		{ AssetItemType.Shader, null }
	};

    private readonly CancellationToken _filePickerToken;
}

