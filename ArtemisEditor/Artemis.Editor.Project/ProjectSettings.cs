using Artemis.Editor.Interfaces;

using Newtonsoft.Json;

namespace Artemis.Editor.Project
{
    // All the code in this file is included in all platforms.
    public class ProjectSettings : IProjectSettings
    {
        public Version Version { get; private set; } = new Version(0, 1);
        public string ProjectName { get; set; } = string.Empty;
        public string AbsoluteProjectDirectory { get; set; } = string.Empty;

        public event IProjectSettings.ProjectLoadedDelegate OnProjectLoad;

        public ProjectSettings() 
        {
        }

        event IProjectSettings.ProjectLoadedDelegate IProjectSettings.OnProjectLoad
        {
            add
            {
                OnProjectLoad += value;
            }

            remove
            {
                OnProjectLoad -= value;
            }
        }

        public void Create(string absolutePath, string projectName)
        {
            ProjectName = projectName;
            AbsoluteProjectDirectory = Path.Combine(absolutePath, projectName);

            if(!Directory.Exists(AbsoluteProjectDirectory))
            {
                Directory.CreateDirectory(AbsoluteProjectDirectory);
            }

            string json = JsonConvert.SerializeObject(this);
            string filename = $"{AbsoluteProjectDirectory}\\{ProjectName}.art{AssetItemType.Project}";

            using FileStream fs = new(filename, FileMode.Create, FileAccess.Write);
            using StreamWriter sr = new(fs);
            sr.Write(json);

            OnProjectLoad?.Invoke(this);
        }

        public bool Load(string absolutePathToProjectFile)
        {
            using FileStream fs = new(absolutePathToProjectFile, FileMode.Open, FileAccess.Read);
            using StreamReader sr = new(fs);

            ProjectSettings settings = JsonConvert.DeserializeObject<ProjectSettings>(sr.ReadToEnd());
            if(settings is null)
            {
                return false;
            }

            Version = settings.Version;
            ProjectName = settings.ProjectName;
            AbsoluteProjectDirectory = settings.AbsoluteProjectDirectory;

            OnProjectLoad?.Invoke(this);

            return true;
        }
    }
}