using Artemis.Editor.Interfaces;

namespace Artemis.App;

public partial class AppShell : Shell
{
	public AppShell()
	{
		InitializeComponent();
        this.Loaded += AppShell_Loaded;
	}

    private void AppShell_Loaded(object sender, EventArgs e)
    {
        this.Handler.MauiContext.Services.GetService<IProjectSettings>()
            .OnProjectLoad += (projectSettings) => this.Title = $"Artemis Editor - {projectSettings.ProjectName}";
    }
}
