#pragma once

#include "Helpers/d3dx12.h"

#include "GpuResourceDx12.h"

namespace Artemis::Renderer::Interfaces
{
    class IGraphicsDevice;
    class ICommandList;
    class IDescriptorHeap;
}

namespace Artemis::Renderer::Device::Dx12
{
    class CommandListDx12;

    class TextureCubeResourceDx12 final : public GpuResourceDx12
    {
    public:		
        TextureCubeResourceDx12(const wchar_t* _pWstrFilename, const bool _bIsDds, const Interfaces::IGraphicsDevice* _pDevice, Interfaces::ICommandList* _pCmdList, Interfaces::IDescriptorHeap* _pTargetSrvHeap, const wchar_t* _pDebugName = nullptr);
        ~TextureCubeResourceDx12(void) override;

    private:
        bool CreateFromDds(const wchar_t* _pWstrFilename, ID3D12Device* _pDevice, CommandListDx12* _pCmdList);
        bool CreateFromWic(const wchar_t* _pWstrFilename, ID3D12Device* _pDevice, CommandListDx12* _pCmdList);
    };
}