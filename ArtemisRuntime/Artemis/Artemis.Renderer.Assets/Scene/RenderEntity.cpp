#include "RenderEntity.h"

#include <vector>

#include "Loading/AssimpLoader.h"

#include "Vectors/Vector3.h"

//#include "Interfaces/ISamplerState.h"
#include "Interfaces/IGraphicsDevice.h"

using namespace DirectX;

namespace Artemis::Renderer::Assets
{
    struct BasicVertex
    {
        Maths::Vector3 pos;
    };

    RenderEntity::RenderEntity() :
        m_xm3Position(),
        m_xm3Rotation(),
        m_fScale(0),
        m_RotationTheta(0),
        m_pModel(nullptr),
        m_matWorld(),
        m_pConstantBuffer(nullptr)
    {
    }

    bool RenderEntity::LoadModelFromFile(const char* _pFilename, Interfaces::IGraphicsDevice* _pDevice, Interfaces::ICommandList* _pCmdList)
    {
        m_pModel = nullptr;
        if (!AssimpLoader::LoadModel(_pDevice, _pCmdList, _pFilename, &m_pModel))
            return false;

        return true;
    }

    bool RenderEntity::LoadModelFromPrimitive(Interfaces::IGraphicsDevice* _pDevice, Interfaces::ICommandList* _pCmdList, int latLongLines)
    {
        int numSphereVertices = ((latLongLines - 2) * latLongLines) + 2;
        int numSphereFaces = ((latLongLines - 3) * (latLongLines) * 2) + (latLongLines * 2);

        float sphereYaw = 0.0f;
        float spherePitch = 0.0f;

        std::vector<BasicVertex> vertices(numSphereVertices);

        XMVECTOR currVertPos = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

        vertices[0].pos.x = 0.0f;
        vertices[0].pos.y = 0.0f;
        vertices[0].pos.z = 1.0f;

        for (DWORD i = 0; i < latLongLines - 2; ++i)
        {
            spherePitch = (i + 1) * (3.14 / (latLongLines - 1));
            auto Rotationx = XMMatrixRotationX(spherePitch);
            for (DWORD j = 0; j < latLongLines; ++j)
            {
                sphereYaw = j * (6.28 / (latLongLines));
                auto Rotationy = XMMatrixRotationZ(sphereYaw);
                currVertPos = XMVector3TransformNormal(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), (Rotationx * Rotationy));
                currVertPos = XMVector3Normalize(currVertPos);
                vertices[i * latLongLines + j + 1].pos.x = XMVectorGetX(currVertPos);
                vertices[i * latLongLines + j + 1].pos.y = XMVectorGetY(currVertPos);
                vertices[i * latLongLines + j + 1].pos.z = XMVectorGetZ(currVertPos);
            }
        }

        vertices[numSphereVertices - 1].pos.x = 0.0f;
        vertices[numSphereVertices - 1].pos.y = 0.0f;
        vertices[numSphereVertices - 1].pos.z = -1.0f;

        std::vector<DWORD> indices(numSphereFaces * 3);

        int k = 0;
        for (DWORD l = 0; l < latLongLines - 1; ++l)
        {
            indices[k] = 0;
            indices[k + 1] = l + 1;
            indices[k + 2] = l + 2;
            k += 3;
        }

        indices[k] = 0;
        indices[k + 1] = latLongLines;
        indices[k + 2] = 1;
        k += 3;

        for (DWORD i = 0; i < latLongLines - 3; ++i)
        {
            for (DWORD j = 0; j < latLongLines - 1; ++j)
            {
                indices[k] = i * latLongLines + j + 1;
                indices[k + 1] = i * latLongLines + j + 2;
                indices[k + 2] = (i + 1) * latLongLines + j + 1;

                indices[k + 3] = (i + 1) * latLongLines + j + 1;
                indices[k + 4] = i * latLongLines + j + 2;
                indices[k + 5] = (i + 1) * latLongLines + j + 2;

                k += 6; // next quad
            }

            indices[k] = (i * latLongLines) + latLongLines;
            indices[k + 1] = (i * latLongLines) + 1;
            indices[k + 2] = ((i + 1) * latLongLines) + latLongLines;

            indices[k + 3] = ((i + 1) * latLongLines) + latLongLines;
            indices[k + 4] = (i * latLongLines) + 1;
            indices[k + 5] = ((i + 1) * latLongLines) + 1;

            k += 6;
        }

        for (DWORD l = 0; l < latLongLines - 1; ++l)
        {
            indices[k] = numSphereVertices - 1;
            indices[k + 1] = (numSphereVertices - 1) - (l + 1);
            indices[k + 2] = (numSphereVertices - 1) - (l + 2);
            k += 3;
        }

        indices[k] = numSphereVertices - 1;
        indices[k + 1] = (numSphereVertices - 1) - latLongLines;
        indices[k + 2] = numSphereVertices - 2;

        Mesh* mesh = new Mesh();
        mesh->Indices = indices.size();
        mesh->pVertexBuffer = _pDevice->CreateVertexBufferResource(_pCmdList, numSphereVertices * sizeof(BasicVertex), sizeof(BasicVertex),
            Interfaces::ResourceFlags::ResourceFlag_None, &vertices[0]);
        mesh->pIndexBuffer = _pDevice->CreateIndexBufferResource(_pCmdList, indices.size() * sizeof(unsigned long), sizeof(unsigned long),
            Interfaces::ResourceFlags::ResourceFlag_None, &indices[0]);

        m_pModel = new RenderModel();
        m_pModel->pMeshList = mesh;
        m_pModel->MeshCount = 1;

        return true;
    }

    void RenderEntity::Update()
    {
        float sRotationSpeed = 10.0f * 0.016f;
        //m_RotationTheta += sRotationSpeed;

        // Update the model matrix. 
        float          angle = 25.0f;
        const XMVECTOR rotation = XMVectorSet(1, 0, 0, 0);

        const XMMATRIX s = XMMatrixScaling(m_fScale, m_fScale, m_fScale);
        const XMMATRIX r = XMMatrixRotationAxis(rotation, XMConvertToRadians(m_RotationTheta));
        const XMMATRIX t = XMMatrixTranslation(m_xm3Position.x, m_xm3Position.y, m_xm3Position.z);
        m_matWorld = s * r * t;
    }
}
