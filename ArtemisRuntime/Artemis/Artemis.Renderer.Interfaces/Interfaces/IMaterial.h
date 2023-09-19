#pragma once

#include <map>

namespace Artemis::Renderer::Interfaces
{
    class IShaderStage;

    struct IMaterial
    {
        IShaderStage* m_pVertexShader;
        IShaderStage* m_pPixelShader;

        std::map<std::string, Artemis::Renderer::Interfaces::IGpuResource*> m_mapTextures;
    };
}