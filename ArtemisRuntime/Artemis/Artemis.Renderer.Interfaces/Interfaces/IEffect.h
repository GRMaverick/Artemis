#pragma once

namespace Artemis::Renderer::Interfaces
{
	class IShaderStage;

	class IEffect
	{
	public:
		~IEffect(void) {};

		virtual void SetVertexShader(Interfaces::IShaderStage* _pShader) = 0;
		virtual void SetPixelShader(Interfaces::IShaderStage* _pShader) = 0;

		virtual Interfaces::IShaderStage* GetVertexShader(void) const = 0;
		virtual Interfaces::IShaderStage* GetPixelShader(void) const = 0;

		virtual const char* GetName(void) const = 0;
		virtual const char* GetFilepath(void) const = 0;
	};
}