using Artemis.Editor.ViewModels;

using CommunityToolkit.Maui;

namespace Artemis.Editor.Views;

public partial class EditorMainPage : ContentPage
{
	public EditorMainPage(EditorMainPageViewModel mainPageViewModel)
	{
		InitializeComponent();

		BindingContext = mainPageViewModel;
	}
}

