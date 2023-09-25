#ifndef __RendererD3D12_h__
#define __RendererD3D12_h__

#include <d3d12.h>
#include <string>
#include <vector>
#include <map>

#include "Interfaces/IRenderer.h"

namespace Artemis
{
	namespace Renderer
	{
		namespace Assets
		{
			class RenderEntity;
			class Camera;
		}
	}
}

namespace Artemis::Core
{
	class GameWindow;
}

namespace Artemis::Renderer::Interfaces
{
	struct IMaterial;
	class IGpuResource;
	class IDescriptorHeap;
	class ISwapChain;
	class ICommandList;
	class ICommandQueue;
	class IGraphicsDevice;
}

struct ConstantBuffer_DLight;
//struct Spotlight;

namespace Artemis::Renderer::Techniques
{
	namespace Scene
	{
		class Camera;
		class RenderEntity;

		// TODO: Consolidate to Render Entity.
		class Skybox
		{
		public:
			Skybox() { }

            void SetName(const char* _pName) { _snprintf_s(m_pModelName, ARRAYSIZE(m_pModelName), "%s", _pName); }
            void SetMaterial(const char* _pMaterialName) { _snprintf_s(m_pMaterialName, ARRAYSIZE(m_pMaterialName), "%s", _pMaterialName); }
            const char* GetName(void) const { return m_pModelName; }
            const char* GetMaterialName(void) const { return m_pMaterialName; }

		protected:
        private:
            char                      m_pMaterialName[64] = {};
            char                      m_pModelName[64] = {};
		};
	}

	class ForwardRenderer final : public Interfaces::IRenderer
	{
	public:
		ForwardRenderer( void );
		~ForwardRenderer( void ) override;

		virtual bool Initialise( Artemis::Core::GameWindow* _pWindow ) override final;
		virtual void Update( double _deltaTime ) override final;
		virtual bool Render( void ) override final;

	private:
		bool m_bNewModelsLoaded;

		Artemis::Renderer::Interfaces::IGraphicsDevice* m_pDevice      = nullptr;
		Artemis::Renderer::Interfaces::ICommandQueue*   m_pGfxCmdQueue = nullptr;
		Artemis::Renderer::Interfaces::ICommandQueue*   m_pCpyCmdQueue = nullptr;

		Artemis::Renderer::Interfaces::ISwapChain* m_pSwapChain;

		//Spotlight* m_spotlight;

		Artemis::Renderer::Interfaces::IDescriptorHeap* m_pImGuiSrvHeap;

		Interfaces::IGpuResource* m_pLightsCb;
		Interfaces::IGpuResource* m_pMainPassCb;
		//Interfaces::IGpuResource* m_pSpotlightCb;

		Artemis::Renderer::Assets::RenderEntity* m_pSkybox;
		std::vector<ConstantBuffer_DLight*>										m_vpLights;
		std::vector<Artemis::Renderer::Assets::Camera*>							m_vpCameras;
		std::vector<Artemis::Renderer::Assets::RenderEntity*>					m_vpRenderEntities;
		std::map<std::string, Artemis::Renderer::Interfaces::IMaterial*>		m_mapMaterials;

		bool LoadContent();
		bool LoadScene( const std::string& _sceneFile );

		void UpdatePassConstants() const;

		void MainRenderPass( const Interfaces::ICommandList* _pGfxCmdList ) const; 
		void RenderCubemap(const Interfaces::ICommandList* _pGfxCmdList) const;

		void ImGuiPass( const Interfaces::ICommandList* _pGfxCmdList ) const;
		void ImGui_MainCamera(void) const;
		void ImGui_Memory(void) const;
		void ImGui_Lights(void) const;
		void ImGui_Objects(void) const;
		void ImGui_DeviceStats(void) const;
		void ImGui_Shaders(void) const;
	};
}

#endif // __RendererD3D12_h__
