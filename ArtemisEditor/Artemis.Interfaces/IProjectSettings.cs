namespace Artemis.Editor.Interfaces
{
    // All the code in this file is included in all platforms.
    public interface IProjectSettings
    {
        Version Version { get; }
        string ProjectName { get; set; }
        string AbsoluteProjectDirectory { get; set; }

        event ProjectLoadedDelegate OnProjectLoad;
        delegate void ProjectLoadedDelegate(IProjectSettings projectSettings);

        void Create(string absolutePath, string projectName);
        bool Load(string absolutePathToProjectFile);
    }
}