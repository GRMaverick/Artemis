using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Artemis.Editor.Services
{
    using Settings;

    internal class AppSettingsService : ISettingsService
    {
        public string SourceAssetDirectory { get; set; } = string.Empty;
    }
}
