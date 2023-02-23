using CommunityToolkit.Maui;
using CommunityToolkit.Maui.Core;
using CommunityToolkit.Maui.Markup;
using Microsoft.Extensions.Logging;

using Newtonsoft.Json;

using Artemis.App.Services;
using Artemis.Editor;
using Artemis.Editor.AssetBrowser.Views;
using Artemis.Editor.AssetBrowser.ViewModels;
using Artemis.Editor.Settings;
using Artemis.Editor.Views;
using Artemis.Editor.ViewModels;

namespace Artemis.App;

public static class MauiProgram
{
    public static MauiApp CreateMauiApp()
    {
        var builder = MauiApp.CreateBuilder();
        builder
            .UseMauiApp<ArtemisApp>()
            .UseMauiCommunityToolkit()
            .UseMauiCommunityToolkitCore()
            .UseMauiCommunityToolkitMarkup()
            .ConfigureFonts(fonts =>
            {
                fonts.AddFont("OpenSans-Regular.ttf", "OpenSansRegular");
                fonts.AddFont("OpenSans-Semibold.ttf", "OpenSansSemibold");
            })
            .RegisterAppServices()
            .RegisterViewModels()
            .RegisterViews();

#if DEBUG
        builder.Logging.AddDebug();
#endif

        return builder.Build();
    }

    public static MauiAppBuilder RegisterAppServices(this MauiAppBuilder appBuilder)
    {
        _ = appBuilder.Services
            //.AddSingleton<>
            .AddSingleton<ISettingsService, AppSettingsService>(sp => {
                string strExeFilePath = System.AppContext.BaseDirectory;
                string strWorkPath = Path.GetDirectoryName(strExeFilePath);

                using FileStream fs = new($@"{strWorkPath}/AppSettings.json", FileMode.Open, FileAccess.Read);
                using StreamReader sr = new(fs);

                string json = sr.ReadToEnd();

                return JsonConvert.DeserializeObject<AppSettingsService>(json);
            });

        return appBuilder;
    }

    public static MauiAppBuilder RegisterViews(this MauiAppBuilder appBuilder)
    {
        appBuilder.Services
            .AddTransient<AppShell>()
            .AddTransient<EditorMainPage>()
            .AddTransient<AssetListView>();

        return appBuilder;
    }

    public static MauiAppBuilder RegisterViewModels(this MauiAppBuilder appBuilder)
    {
        appBuilder.Services
            .AddTransient<EditorMainPageViewModel>()
            .AddSingleton<IAssetListViewModel, AssetListViewModel>();

        return appBuilder;
    }
}
