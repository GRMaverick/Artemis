#pragma once

#include <map>

namespace Artemis::Renderer::Interfaces
{
    class IEffect;

    struct IMaterial
    {
        IEffect* Effect;

        std::map<std::string, Artemis::Renderer::Interfaces::IGpuResource*> m_mapTextures;
    };
}