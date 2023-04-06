using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace Artemis.Editor.Controls.ViewModels
{
    public class DestinationViewViewModel : INotifyPropertyChanged
    {
        public string FiletypeAsString
        {
            get
            {
                return _filetypeAsString;
            }
            set
            {
                if (_filetypeAsString != value)
                {
                    _filetypeAsString = value;
                    OnPropertyChanged(nameof(FiletypeAsString));
                }
            }
        }

        public string SelectedLocation
        {
            get
            {
                return _selectedLocation;
            }
            set
            {
                if (_selectedLocation != value)
                {
                    _selectedLocation = value;
                    OnPropertyChanged(nameof(SelectedLocation));
                }
            }
        }

        public DestinationViewViewModel() { }
        public DestinationViewViewModel(string targetAsString)
        {
            FiletypeAsString = $".art{targetAsString}";
        }

        public event PropertyChangedEventHandler PropertyChanged;

        private void OnPropertyChanged(string name) => 
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));

        private string _filetypeAsString = string.Empty;
        private string _selectedLocation = string.Empty;
    }
}
