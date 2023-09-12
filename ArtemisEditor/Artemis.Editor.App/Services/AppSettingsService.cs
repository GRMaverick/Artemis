namespace Artemis.Editor.App.Services
{
    using Artemis.Editor.Interfaces;

    internal class AppSettingsService : ISettingsService
    {
        public string SourceAssetDirectory { get; set; } = string.Empty;
    }
}
