//#include "Defines.h"
#define ARRAY_LENGTH(a) (sizeof(a) / sizeof(*(a)))

#include "ForwardRenderer.h"

#include <assert.h>
#include <DirectXMath.h>
#include <fstream>

#include "Factories/CommandQueueFactory.h"
#include "Factories/GraphicsDeviceFactory.h"
#include "Factories/CommandListFactory.h"

#include "Interfaces/ISwapChain.h"
#include "Interfaces/ICommandList.h"
#include "Interfaces/IBufferResource.h"
#include "Interfaces/IMaterial.h"
#include "Interfaces/IBufferResource.h"

#include "Loaders/CLParser.h"

#include <rapidxml/rapidxml_print.hpp>
#include <rapidxml/rapidxml_utils.hpp>

#include "Memory/MemoryGlobalTracking.h"

#include "Loading/AssimpLoader.h"
#include "Scene/Camera.h"
#include "Scene/RenderEntity.h"

#include "Cache/Effect.h"
#include "Cache/ShaderCache.h"
#include "Constants/ConstantTable.h"

#include "Tables/SamplerTable.h"

#include "Debug/ImGUIEngine.h"
#include "Helpers/ProfileMarker.h"
#include <filesystem>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "DirectXTK12.lib")

using namespace DirectX;
using namespace Microsoft::WRL;

using namespace Artemis::Core;
using namespace Artemis::Maths;
using namespace Artemis::Memory;
using namespace Artemis::Utilities;
using namespace Artemis::Renderer::Shaders;
using namespace Artemis::Renderer::Interfaces;

#define PRAGMA_TODO(todo)	__pragma(message("[TODO]: "todo));
#define BACK_BUFFERS 2

namespace Artemis::Renderer::Techniques
{
	Interfaces::BlendDesc DefaultBlendDesc()
	{
		Interfaces::BlendDesc desc;
		desc.AlphaToCoverageEnable  = false;
		desc.IndependentBlendEnable = false;

		constexpr Interfaces::RenderTargetBlendDesc rtbDesc = {false, false, Interfaces::Blend_One, Interfaces::Blend_Zero, Interfaces::BlendOp_Add, Interfaces::Blend_One, Interfaces::Blend_Zero, Interfaces::BlendOp_Add, Interfaces::LogicOp_NoOp, Interfaces::ColourWriteEnable_All};
		for ( unsigned int i = 0; i < ARRAYSIZE( desc.RenderTarget ); ++i )
		{
			desc.RenderTarget[i] = rtbDesc;
		}

		return desc;
	}

	Interfaces::RasteriserStateDesc DefaultRasteriserStateDesc()
	{
		Interfaces::RasteriserStateDesc desc;
		desc.FillMode              = Interfaces::FillMode_Solid;
		desc.CullMode              = Interfaces::CullMode_Back;
		desc.FrontCounterClockwise = false;
		desc.DepthBias             = 0;
		desc.DepthBiasClamp        = 0.0f;
		desc.SlopeScaledDepthBias  = 0.0f;
		desc.DepthClipEnable       = true;
		desc.MultisampleEnable     = false;
		desc.AntialiasedLineEnable = false;
		desc.ForcedSampleCount     = 0;
		desc.ConservativeRaster    = Interfaces::ConservativeRasterisationMode_On;
		return desc;
	}

	Interfaces::DepthStencilDesc DefaultDepthStencilDesc()
	{
		Interfaces::DepthStencilDesc desc;
		desc.DepthEnable                                          = true;
		desc.DepthWriteMask                                       = Interfaces::DepthWriteMask_All;
		desc.DepthFunc                                            = Interfaces::ComparisonFunc_Less;
		desc.StencilEnable                                        = false;
		desc.StencilReadMask                                      = 0xFF;
		desc.StencilWriteMask                                     = 0xFF;
		constexpr Interfaces::DepthStencilOpDesc defaultStencilOp = {Interfaces::StencilOp_Keep, Interfaces::StencilOp_Keep, Interfaces::StencilOp_Keep, Interfaces::ComparisonFunc_Always};
		desc.FrontFace                                            = defaultStencilOp;
		desc.BackFace                                             = defaultStencilOp;
		return desc;
	}

	ForwardRenderer::ForwardRenderer( void ):
		m_bNewModelsLoaded( false ),
		m_pImGuiSrvHeap( nullptr ),
		m_pLightsCb( nullptr ),
		m_pMainPassCb( nullptr )//,
		//m_pSpotlightCb( nullptr )
	{
		m_pSwapChain = nullptr;
	}

	ForwardRenderer::~ForwardRenderer( void )
	{
		if ( m_pSwapChain )
			delete m_pSwapChain;
		m_pSwapChain = nullptr;
	}

	bool ForwardRenderer::Initialise( GameWindow* _pWindow )
	{
		m_pDevice = Artemis::Renderer::Device::Factories::GraphicsDeviceFactory::Construct( CLParser::Instance()->HasArgument( "d3ddebug" ) );

		m_pGfxCmdQueue = Artemis::Renderer::Device::Factories::CommandQueueFactory::Construct();
		m_pDevice->CreateCommandQueue( Interfaces::CommandListType_Direct, &m_pGfxCmdQueue, L"Gfx" );

		m_pCpyCmdQueue = Artemis::Renderer::Device::Factories::CommandQueueFactory::Construct();
		m_pDevice->CreateCommandQueue( Interfaces::CommandListType_Copy, &m_pCpyCmdQueue, L"Cpy" );

		if ( !m_pDevice->CreateSwapChain( &m_pSwapChain, m_pGfxCmdQueue, _pWindow, BACK_BUFFERS, L"Swap Chain" ) )
			return false;

		Artemis::Renderer::Shaders::ShaderCache::Instance()->LoadCache( std::string(CLParser::Instance()->GetArgument("data")) + std::string("\\Shaders\\*") );

		if ( !m_pDevice->CreateDescriptorHeap( Interfaces::DescriptorHeapType_CbvSrvUav, &m_pImGuiSrvHeap, Interfaces::DescriptorHeapFlags_ShaderVisible, 1, L"ImGUI SRV" ) )
			return false;

		Tables::SamplerTable::Instance()->CreateSamplerState("PointWrap", SamplerStateFilter::EPoint, SamplerStateWrapMode::EWrap, SamplerStateComparisonFunction::EAlways, m_pDevice);
		Tables::SamplerTable::Instance()->CreateSamplerState("PointClamp", SamplerStateFilter::EPoint, SamplerStateWrapMode::EClamp, SamplerStateComparisonFunction::EAlways, m_pDevice);
		Tables::SamplerTable::Instance()->CreateSamplerState("LinearWrap", SamplerStateFilter::ELinear, SamplerStateWrapMode::EWrap, SamplerStateComparisonFunction::EAlways, m_pDevice);
		Tables::SamplerTable::Instance()->CreateSamplerState("LinearClamp", SamplerStateFilter::ELinear, SamplerStateWrapMode::EClamp, SamplerStateComparisonFunction::EAlways, m_pDevice);
		Tables::SamplerTable::Instance()->CreateSamplerState("AnisotropicWrap", SamplerStateFilter::EAnisotropic, SamplerStateWrapMode::EWrap, SamplerStateComparisonFunction::EAlways, m_pDevice);
		Tables::SamplerTable::Instance()->CreateSamplerState("AnisotropicClamp", SamplerStateFilter::EAnisotropic, SamplerStateWrapMode::EClamp, SamplerStateComparisonFunction::EAlways, m_pDevice);

		ImGUIEngine::Initialise( _pWindow, m_pDevice, m_pImGuiSrvHeap );

		if ( !LoadContent() )
			return false;

		return true;
	}

	struct ModelDefinition
	{
		const char* MeshFilename = nullptr;
		const char* ObjectName   = nullptr;
		const char* MaterialName = nullptr;
		float       Position[3]  = {0};
		float       Scale        = 1.0f;
	};
	
	const char* GetExtension(const char* _pFilename)
	{
		size_t       extensionStart = 0;
		const size_t size = strlen(_pFilename);
		for (size_t i = size - 1; i >= 0; --i)
		{
			if (_pFilename[i] == '.')
			{
				extensionStart = i + 1;
				break;
			}
		}

		char* ext = new char[5];
		snprintf(ext, 5, "%s", &_pFilename[extensionStart]);
		return ext;
	}

	bool ForwardRenderer::LoadScene( const std::string& _sceneFile )
	{
		if(std::filesystem::exists(_sceneFile.c_str()))
		{
			rapidxml::file<>         xmlFile(_sceneFile.c_str());
			rapidxml::xml_document<> doc;
			doc.parse<0>(xmlFile.data());

			const rapidxml::xml_node<>* root = doc.first_node("Root");
			const rapidxml::xml_node<>* cameras = root->first_node("Cameras");
			const rapidxml::xml_node<>* camera = cameras->first_node("Camera");
			while (camera != nullptr)
			{
				Artemis::Renderer::Assets::Camera* pCamera = new Artemis::Renderer::Assets::Camera();
				const rapidxml::xml_node<>* posNode = camera->first_node("Position");
				const rapidxml::xml_node<>* tarNode = camera->first_node("Target");
				const rapidxml::xml_node<>* upNode = camera->first_node("Up");
				const rapidxml::xml_node<>* fovNode = camera->first_node("FOV");

				float x = static_cast<float>(atof(posNode->first_attribute("X")->value()));
				float y = static_cast<float>(atof(posNode->first_attribute("Y")->value()));
				float z = static_cast<float>(atof(posNode->first_attribute("Z")->value()));
				pCamera->SetPosition(x, y, z);

				x = static_cast<float>(atof(tarNode->first_attribute("X")->value()));
				y = static_cast<float>(atof(tarNode->first_attribute("Y")->value()));
				z = static_cast<float>(atof(tarNode->first_attribute("Z")->value()));
				pCamera->SetTarget(x, y, z);

				x = static_cast<float>(atof(upNode->first_attribute("X")->value()));
				y = static_cast<float>(atof(upNode->first_attribute("Y")->value()));
				z = static_cast<float>(atof(upNode->first_attribute("Z")->value()));
				pCamera->SetUp(x, y, z);

				x = static_cast<float>(atof(fovNode->first_attribute("Value")->value()));
				y = static_cast<float>(atof(CLParser::Instance()->GetArgument("Width")));
				z = static_cast<float>(atof(CLParser::Instance()->GetArgument("Height")));
				pCamera->SetFieldOfView(x);
				pCamera->SetAspectRatio(y / z);

				m_vpCameras.push_back(pCamera);

				camera = camera->next_sibling("Camera");
			}

			const rapidxml::xml_node<>* lights = root->first_node("Lights");
			const rapidxml::xml_node<>* light = lights->first_node("Light");
			while (light != nullptr)
			{
				ConstantBuffer_DLight* pLight = new ConstantBuffer_DLight();
				const rapidxml::xml_node<>* posNode = light->first_node("Position");
				const rapidxml::xml_node<>* diffuse = light->first_node("Diffuse");

				const float x = static_cast<float>(atof(posNode->first_attribute("X")->value()));
				const float y = static_cast<float>(atof(posNode->first_attribute("Y")->value()));
				const float z = static_cast<float>(atof(posNode->first_attribute("Z")->value()));
				pLight->Position = Vector3(x, y, z);

				float r = static_cast<float>(atof(diffuse->first_attribute("R")->value()));
				float g = static_cast<float>(atof(diffuse->first_attribute("G")->value()));
				float b = static_cast<float>(atof(diffuse->first_attribute("B")->value()));
				float a = static_cast<float>(atof(diffuse->first_attribute("A")->value()));
				pLight->Diffuse = Artemis::Maths::Vector4(r, g, b, a);

				m_vpLights.push_back(pLight);

				light = light->next_sibling("Light");
			}

			Interfaces::ICommandList* pList = Artemis::Renderer::Device::Factories::CommandListFactory::Construct();
			m_pDevice->CreateCommandList(Interfaces::ECommandListType::CommandListType_Copy, &pList, L"CpyList");
			pList->Reset();

			const rapidxml::xml_node<>* matInstances = root->first_node("MaterialInstances");
			const rapidxml::xml_node<>* matInstance = matInstances->first_node("MaterialInstance");
			while (matInstance != nullptr)
			{				
				Artemis::Renderer::Interfaces::IEffect* effect = Artemis::Renderer::Shaders::ShaderCache::Instance()->GetEffect(matInstance->first_node("Effect")->first_attribute("Name")->value());

				Artemis::Renderer::Interfaces::IMaterial* mat = new Artemis::Renderer::Interfaces::IMaterial();
				mat->Effect = effect;

				const rapidxml::xml_node<>* textures = matInstance->first_node("Textures");
				const rapidxml::xml_node<>* texture = textures->first_node("Texture");
				while (texture != nullptr)
				{
					std::string strFilename = CLParser::Instance()->GetArgument("data") + std::string("\\Textures\\") + "\\" + std::string(texture->first_attribute("Path")->value());
					const std::wstring wstrFilename = std::wstring(strFilename.begin(), strFilename.end());
                    const char* ext = GetExtension(texture->first_attribute("Path")->value());
                    char* dimension = texture->first_attribute("Dimension")->value();

					IGpuResource* pTexture = nullptr;
					if (strncmp(ext, "dds", 3) == 0)
					{
						if (strstr(dimension, "Cube"))
                        {
                            pTexture = m_pDevice->CreateTextureCube(wstrFilename.c_str(), pList, wstrFilename.c_str());
						}
						else
                        {
                            pTexture = m_pDevice->CreateTexture2D(wstrFilename.c_str(), pList, wstrFilename.c_str());
						}
					}
					else
                    {
                        if (strstr(dimension, "Cube"))
                        {
                            pTexture = m_pDevice->CreateWicTextureCube(wstrFilename.c_str(), pList, wstrFilename.c_str());
                        }
                        else
                        {
                            pTexture = m_pDevice->CreateWicTexture2D(wstrFilename.c_str(), pList, wstrFilename.c_str());
                        }
					}

					std::string texReg = std::string(texture->first_attribute("Register")->value());
					mat->m_mapTextures.emplace(texReg, pTexture);

					auto samplerAttr = texture->first_attribute("Sampler");
					if (samplerAttr)
					{
						std::string sampler = std::string(samplerAttr->value());
						if (sampler.empty())
						{
							sampler = "PointWrap";
						}
						mat->m_mapSamplers.emplace(texReg, sampler);
					}
					else
					{
						mat->m_mapSamplers.emplace(texReg, "PointWrap");
					}

					texture = texture->next_sibling("Texture");
				}

				m_mapMaterials.emplace(matInstance->first_attribute("Name")->value(), mat);
				matInstance = matInstance->next_sibling("MaterialInstance");
			}

			const rapidxml::xml_node<>* instances = root->first_node("ModelInstances");
			const rapidxml::xml_node<>* instance = instances->first_node("Instance");
			while (instance != nullptr)
			{
				Artemis::Renderer::Assets::RenderEntity* pInstance = new Artemis::Renderer::Assets::RenderEntity();

				const std::string pModelPath = CLParser::Instance()->GetArgument("data") + std::string("\\Models\\") + std::string(instance->first_attribute("ModelPath")->value());
				pInstance->SetModelName(instance->first_attribute("ModelName")->value());
				pInstance->LoadModelFromFile(pModelPath.c_str(), m_pDevice, pList);
				pInstance->SetMaterial(instance->first_attribute("MaterialInstance")->value());
				pInstance->SetConstantBuffer(Artemis::Renderer::Shaders::ConstantTable::Instance()->CreateConstantBuffer("ObjectCB", m_pDevice));
				pInstance->SetRotation(0.0f, 0.0f, 0.0f);

				const rapidxml::xml_node<>* transform = instance->first_node("Transform");
				const float                 x = static_cast<float>(atof(transform->first_attribute("PosX")->value()));
				const float                 y = static_cast<float>(atof(transform->first_attribute("PosY")->value()));
				const float                 z = static_cast<float>(atof(transform->first_attribute("PosZ")->value()));
				const float                 scale = static_cast<float>(atof(transform->first_attribute("Scale")->value()));
				pInstance->SetScale(scale);
				pInstance->SetPosition(x, y, z);

				m_vpRenderEntities.push_back(pInstance);

				instance = instance->next_sibling("Instance");
			}

			const rapidxml::xml_node<>* skybox = root->first_node("SkyBox");
			if (skybox != nullptr)
			{
				m_pSkybox = new Artemis::Renderer::Assets::RenderEntity();
				m_pSkybox->SetModelName(skybox->first_attribute("Name")->value());
				m_pSkybox->SetMaterial(skybox->first_attribute("MaterialInstance")->value());
				m_pSkybox->LoadModelFromPrimitive(m_pDevice, pList, 10);
				m_pSkybox->SetConstantBuffer(Artemis::Renderer::Shaders::ConstantTable::Instance()->CreateConstantBuffer("ObjectCB", m_pDevice));
				m_pSkybox->SetScale(1.0f);
			}

			m_pCpyCmdQueue->SubmitToQueue(pList);

			return true;
		}

		return false;
	}

	bool ForwardRenderer::LoadContent( void )
	{
		m_bNewModelsLoaded = true;

		m_pMainPassCb  = Artemis::Renderer::Shaders::ConstantTable::Instance()->CreateConstantBuffer( "PassCB", m_pDevice );
		m_pLightsCb    = Artemis::Renderer::Shaders::ConstantTable::Instance()->CreateConstantBuffer( "DirectionalLightCB", m_pDevice );
		//m_pSpotlightCb = Artemis::Renderer::Shaders::ConstantTable::Instance()->CreateConstantBuffer( "SpotlightCB", m_pDevice );

		LogInfo( "Loading Scene:" );

		PRAGMA_TODO("Command Line Arg for Scene Root");
		if (!CLParser::Instance()->HasArgument("scene") || !CLParser::Instance()->HasArgument("data"))
		{
			LogError("No Scene File Defined in CLParser");
			return false;
		}

		const std::string strScenePath = std::string(CLParser::Instance()->GetArgument("data")) +
			std::string("\\Scenes\\") +
			std::string(CLParser::Instance()->GetArgument("scene"));
		if ( !LoadScene(strScenePath))
		{
			LogError( "Scene Loading Failed" );
			return false;
		}

		m_pCpyCmdQueue->ExecuteCommandLists();
		m_pCpyCmdQueue->Flush();

		return true;
	}

	float direction = 2;
	void ForwardRenderer::Update( double _deltaTime )
	{
		UpdatePassConstants();

		{
			// Skybox
			Vector3 pos = m_vpCameras[0]->GetPosition();
			if (m_pSkybox)
			{
				m_pSkybox->SetPosition(pos.x, pos.y, pos.z);
				m_pSkybox->Update();

				ConstantBuffer_Object skyboxCb;
				skyboxCb.World = m_pSkybox->GetWorld();
				m_pSkybox->GetConstantBuffer()->UpdateValue("World", &skyboxCb, sizeof(ConstantBuffer_Object));
			}
		}

		for ( UINT i = 0; i < m_vpRenderEntities.size(); ++i )
		{
			{
				static const float sRotationSpeed = 5.0f * 0.016f;
				Vector3 rotation = Vector3(m_vpRenderEntities[i]->GetRotation().x, m_vpRenderEntities[i]->GetRotation().y, m_vpRenderEntities[i]->GetRotation().z);
				rotation += Vector3(1.0f, 1.0f, 0.0f) * sRotationSpeed;
				m_vpRenderEntities[i]->SetRotation(rotation.x, rotation.y, rotation.z);
			}

			m_vpRenderEntities[i]->Update();

			ConstantBuffer_Object obj;
			obj.World = m_vpRenderEntities[i]->GetWorld();

			if ( m_vpRenderEntities[i]->GetConstantBuffer() )
				bool bRet = m_vpRenderEntities[i]->GetConstantBuffer()->UpdateValue( "World", &obj, sizeof( ConstantBuffer_Object ) );
		}
	}

	void ForwardRenderer::UpdatePassConstants() const
	{
		m_vpCameras[0]->Update();

		ConstantBuffer_Pass cbPass           = ConstantBuffer_Pass();
		cbPass.EyePosition    = m_vpCameras[0]->GetPosition();
		cbPass.ViewProjection = m_vpCameras[0]->GetView() * m_vpCameras[0]->GetProjection();

		bool bRet = false;
		if ( m_pMainPassCb )
			bRet = m_pMainPassCb->UpdateValue( nullptr, &cbPass, sizeof( ConstantBuffer_Pass ) );

		if (m_pLightsCb)
		{
			ConstantBuffer_DLight* lights = new ConstantBuffer_DLight[m_vpLights.size()];
			for (int i = 0; i < m_vpLights.size(); ++i)
			{
				lights[i].Diffuse = m_vpLights[i]->Diffuse;
				lights[i].Position = m_vpLights[i]->Position;
			}
			bRet = m_pLightsCb->UpdateValue(nullptr, lights, m_vpLights.size() * sizeof(ConstantBuffer_DLight));
		}
		//if ( m_pSpotlightCb )
		//	bRet = m_pSpotlightCb->UpdateValue( nullptr, nullptr, sizeof( Spotlight ) );
	}

	bool ForwardRenderer::Render( void )
	{
		m_pDevice->BeginFrame();

		Interfaces::ICommandList* pGfxCmdList = m_pDevice->GetImmediateContext();

		// Rendering
		{
			Artemis::Renderer::Helpers::RenderMarker profile( pGfxCmdList, "Render" );

			m_pSwapChain->PrepareForRendering( pGfxCmdList );
			m_pSwapChain->SetOmRenderTargets( pGfxCmdList );
			pGfxCmdList->SetIaPrimitiveTopology( Interfaces::PrimitiveTopology_TriangleList );

			RenderCubemap( pGfxCmdList );
			MainRenderPass( pGfxCmdList );
			ImGuiPass( pGfxCmdList );
		}

		// Presentation
		{
			m_pSwapChain->PrepareForPresentation( pGfxCmdList );
			m_pGfxCmdQueue->SubmitToQueue( pGfxCmdList );
			m_pGfxCmdQueue->ExecuteCommandLists();

			bool bRet = m_pSwapChain->Present();
			m_pGfxCmdQueue->Signal();
			m_pSwapChain->Swap();
			m_pGfxCmdQueue->Wait();
		}

		m_pDevice->EndFrame();

		return true;
	}

    void ForwardRenderer::MainRenderPass(const Interfaces::ICommandList* _pGfxCmdList) const
    {
        Artemis::Renderer::Helpers::RenderMarker profile(_pGfxCmdList, "MainRenderPass");

        m_pDevice->SetBlendState(DefaultBlendDesc());
        m_pDevice->SetRasterizerState(DefaultRasteriserStateDesc());
        m_pDevice->SetDepthStencilState(DefaultDepthStencilDesc());

        // Per Object Draws
        for (UINT i = 0; i < m_vpRenderEntities.size(); ++i)
        {
            const Artemis::Renderer::Assets::RenderEntity* pModel = m_vpRenderEntities[i];
            if (!pModel->GetModel())
            {
                continue;
            }

            Artemis::Renderer::Helpers::RenderMarker profile(_pGfxCmdList, "%s", pModel->GetModelName());

            Interfaces::IGpuResource* pModelCb = pModel->GetConstantBuffer();
            Artemis::Renderer::Interfaces::IMaterial* pModelMat = m_mapMaterials.at(pModel->GetMaterialName());
			m_pDevice->SetMaterial(pModelMat);

			Artemis::Renderer::Interfaces::IMaterial* pSkyboxMat = m_mapMaterials.at(m_pSkybox->GetMaterialName());
			m_pDevice->SetTexture("Skybox", pSkyboxMat->m_mapTextures["Skybox"]);
			m_pDevice->SetSamplerState("Skybox", Tables::SamplerTable::Instance()->GetSamplerState(pSkyboxMat->m_mapSamplers["Skybox"].c_str()));

			for (auto iter = pModelMat->m_mapSamplers.begin(); iter != pModelMat->m_mapSamplers.end(); ++iter)
			{
				m_pDevice->SetSamplerState(iter->first.c_str(), Tables::SamplerTable::Instance()->GetSamplerState(iter->second.c_str()));
			}

            m_pDevice->SetConstantBuffer("ObjectCB", pModelCb);
            m_pDevice->SetConstantBuffer("PassCB", m_pMainPassCb);
            m_pDevice->SetConstantBuffer("DirectionalLightCB", m_pLightsCb);
            //m_pDevice->SetConstantBuffer( "SpotlightCB", m_pSpotlightCb );

            for (UINT j = 0; j < pModel->GetModel()->MeshCount; ++j)
            {
                const Artemis::Renderer::Assets::Mesh& rMesh = pModel->GetModel()->pMeshList[j];

                Artemis::Renderer::Helpers::RenderMarker profile(_pGfxCmdList, "Mesh: %llu", j);
                if (m_pDevice->FlushState())
                {
                    _pGfxCmdList->DrawIndexedInstanced(rMesh.pVertexBuffer, rMesh.pIndexBuffer, rMesh.Indices);
                }
            }
        }
    }

	void ForwardRenderer::RenderCubemap(const Interfaces::ICommandList* _pGfxCmdList) const
	{
		static Interfaces::RasteriserStateDesc rs = DefaultRasteriserStateDesc();
		rs.CullMode = Interfaces::CullMode_None;
		m_pDevice->SetRasterizerState(rs);

		static Interfaces::DepthStencilDesc ds = DefaultDepthStencilDesc();
		ds.DepthEnable = true;
		ds.DepthWriteMask = DepthWriteMask_All;
		ds.DepthFunc = ComparisonFunc_LessEqual;
		m_pDevice->SetDepthStencilState(ds);

		if (m_pSkybox == nullptr)
		{
			return;
		}

		Artemis::Renderer::Helpers::RenderMarker profile(_pGfxCmdList, "Cubemap Pass");

		Artemis::Renderer::Interfaces::IMaterial* pMat = m_mapMaterials.at(m_pSkybox->GetMaterialName());
		m_pDevice->SetMaterial(pMat);
		m_pDevice->SetSamplerState("Skybox", m_pDevice->GetDefaultSamplerState());

		Interfaces::IGpuResource* pCubemapCb = m_pSkybox->GetConstantBuffer();
		m_pDevice->SetConstantBuffer("ObjectCB", pCubemapCb);
		m_pDevice->SetConstantBuffer("PassCB", m_pMainPassCb);
		m_pDevice->SetConstantBuffer("DirectionalLightCB", m_pLightsCb);

		for (UINT j = 0; j < m_pSkybox->GetModel()->MeshCount; ++j)
		{
			const Artemis::Renderer::Assets::Mesh& rMesh = m_pSkybox->GetModel()->pMeshList[j];

			Artemis::Renderer::Helpers::RenderMarker profile(_pGfxCmdList, "Mesh: %llu", j);
			if (m_pDevice->FlushState())
			{
				_pGfxCmdList->DrawIndexedInstanced(rMesh.pVertexBuffer, rMesh.pIndexBuffer, rMesh.Indices);
			}
		}
	}

	void ForwardRenderer::ImGuiPass( const Interfaces::ICommandList* _pGfxCmdList ) const
	{
#if defined(_DEBUG)
		Artemis::Renderer::Helpers::RenderMarker profile( _pGfxCmdList, "ImGUI" );

		Artemis::Renderer::Interfaces::IDescriptorHeap* pHeaps[] = {m_pImGuiSrvHeap};
		_pGfxCmdList->SetDescriptorHeaps( pHeaps, _countof( pHeaps ) );

		ImGUIEngine::Begin();

		ImGui_MainCamera();
		ImGui_Memory();
		ImGui_Lights();
		//ImGui_Objects();
		ImGui_DeviceStats();
		ImGui_Shaders();


		ImGUIEngine::End();
		ImGUIEngine::Draw( _pGfxCmdList );
#endif
	}

	void ForwardRenderer::ImGui_MainCamera(void) const
	{
		if (ImGui::Begin("Main Camera"))
		{
			for (unsigned int i = 0; i < m_vpCameras.size(); ++i)
			{
				float v[3];
				v[0] = m_vpCameras[i]->GetPosition().x;
				v[1] = m_vpCameras[i]->GetPosition().y;
				v[2] = m_vpCameras[i]->GetPosition().z;

				if (ImGui::SliderFloat3("Position:", v, -10.0f, 10.0f))
				{
					m_vpCameras[i]->SetPosition(v[0], v[1], v[2]);
				}

				v[0] = m_vpCameras[i]->GetTarget().x;
				v[1] = m_vpCameras[i]->GetTarget().y;
				v[2] = m_vpCameras[i]->GetTarget().z;
				if (ImGui::SliderFloat3("Target:", v, -10.0f, 10.0f))
				{
					m_vpCameras[i]->SetTarget(v[0], v[1], v[2]);
				}

				v[0] = m_vpCameras[i]->GetUp().x;
				v[1] = m_vpCameras[i]->GetUp().y;
				v[2] = m_vpCameras[i]->GetUp().z;
				if (ImGui::SliderFloat3("Up:", v, -10.0f, 10.0f))
				{
					m_vpCameras[i]->SetUp(v[0], v[1], v[2]);
				}
			}
			ImGui::End();
		}
	}

	void ForwardRenderer::ImGui_Memory(void) const
	{
		if (ImGui::Begin("Memory"))
		{
			static constexpr float kMb = 1024 * 1024;
			float                  fTotalMemUsage = 0.0f;
			int                    iTotalAllocations = 0;
			for (unsigned int i = 0; i < static_cast<unsigned int>(MemoryContextCategory::ECategories); ++i)
			{
				const char* pCatName = MemoryGlobalTracking::GetContextName(static_cast<MemoryContextCategory>(i));
				const MemoryContextData& data = MemoryGlobalTracking::GetContextStats(static_cast<MemoryContextCategory>(i));

				const int iNetAllocations = data.Allocations - data.Deallocations;
				ImGui::CollapsingHeader(pCatName, ImGuiTreeNodeFlags_Bullet);
				ImGui::Text("Net Allocations: %i", iNetAllocations);
				ImGui::Text("Allocated Size: %0.3f MB", data.TotalAllocationSize / kMb);

				fTotalMemUsage += data.TotalAllocationSize;
				iTotalAllocations += iNetAllocations;
			}
			ImGui::CollapsingHeader("Total Memory Usage:", ImGuiTreeNodeFlags_Bullet);
			ImGui::Text("Size Allocation: %0.3f MB", fTotalMemUsage / kMb);
			ImGui::Text("Net Allocations: %i", iTotalAllocations);
			ImGui::End();
		}
	}

	void ForwardRenderer::ImGui_Lights(void) const
	{
		if (ImGui::Begin("Lights"))
		{
			for (unsigned int i = 0; i < m_vpLights.size(); ++i)
			{
				float v[3];
				v[0] = m_vpLights[i]->Position.x;
				v[1] = m_vpLights[i]->Position.y;
				v[2] = m_vpLights[i]->Position.z;
				if (ImGui::SliderFloat3("Position:", v, -5.0, 5.0f))
				{
					m_vpLights[i]->Position.x = v[0];
					m_vpLights[i]->Position.y = v[1];
					m_vpLights[i]->Position.z = v[2];
				}

				v[0] = m_vpLights[i]->Diffuse.x;
				v[1] = m_vpLights[i]->Diffuse.y;
				v[2] = m_vpLights[i]->Diffuse.z;
				if (ImGui::SliderFloat3("Diffuse:", v, 0.0f, 1.0f))
				{
					m_vpLights[i]->Diffuse.x = v[0];
					m_vpLights[i]->Diffuse.y = v[1];
					m_vpLights[i]->Diffuse.z = v[2];
				}
			}

			ImGui::End();
		}
	}

	void ForwardRenderer::ImGui_Objects(void) const
	{
		if (ImGui::Begin("Objects:"))
		{
			ImGui::End();
		}
	}

	void ForwardRenderer::ImGui_DeviceStats(void) const
	{
		if (ImGui::Begin("Device Flush State Stats:"))
		{
			const Artemis::Renderer::Interfaces::DeviceStateStats stats = m_pDevice->GetDeviceStats();
			ImGui::Text("Texture Updates: %lu", stats.TextureUpdates);
			ImGui::Text("Shader Updates: %lu", stats.ShaderUpdates);
			ImGui::Text("Render Target Updates: %lu", stats.RenderTargetUpdates);
			ImGui::Text("Depth Buffer Updates: %lu", stats.DepthBufferUpdates);
			ImGui::Text("Constant Buffer Updates: %lu", stats.ConstantBufferUpdates);
			ImGui::Text("Pipeline State Updates: %lu", stats.PipelineStateUpdates);
			ImGui::Text("Root Signature Updates: %lu", stats.RootSignatureUpdates);
			ImGui::End();
		}
	}

	void ForwardRenderer::ImGui_Shaders(void) const
	{
		if (ImGui::Begin("Shader Cache", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			const std::vector<IEffect*>* effects = Shaders::ShaderCache::Instance()->GetLoadedEffects();
			for(int i = 0; i < effects->size(); ++i)
			{
				IEffect* effect = effects->at(i);

				if (ImGui::Button(effect->GetName(), ImVec2(250, 25)))
				{
					Shaders::ShaderCache::Instance()->ReloadEffect(effect->GetName());
				}
				ImGui::NewLine();
			}
			ImGui::End();
		}
	}
}
