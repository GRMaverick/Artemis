namespace Artemis.App.Services
{
    using Artemis.Editor.Settings;

    internal class AppSettingsService : ISettingsService
    {
        public string SourceAssetDirectory { get; set; } = string.Empty;
    }
}
