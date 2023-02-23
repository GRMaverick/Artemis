namespace Artemis.App;

public partial class ArtemisApp : Application
{
	public ArtemisApp()
	{
		InitializeComponent();

		MainPage = new AppShell();
	}
}
