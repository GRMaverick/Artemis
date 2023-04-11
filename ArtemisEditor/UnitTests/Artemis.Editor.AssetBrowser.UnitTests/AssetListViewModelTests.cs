namespace Artemis.Editor.AssetBrowser.UnitTests
{
    using Artemis.Editor.AssetBrowser.ViewModels;
    using Artemis.Editor.Interfaces;
    using Moq;

    [TestClass]
    public class AssetListViewModelTests
    {
        [TestMethod]
        public void TreeEmpty_True()
        {
            // Arrange
            Mock<IProjectSettings> settings = new();
            
            // Act
            IAssetListViewModel assetListViewModel = new AssetListViewModel(settings.Object);

            // Assert
            Assert.IsTrue(assetListViewModel.SelectedNode.Children.Count() == 0, "Root Node has children");
        }
    }
}