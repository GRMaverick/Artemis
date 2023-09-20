#ifndef __ShaderCache_h__
#define __ShaderCache_h__

#include <string>
#include <vector>

#include "Effect.h"

namespace Artemis::Renderer::Interfaces
{
	class IEffect;
	class IShaderStage;
	class IShaderCompiler;
}

namespace Artemis::Renderer::Shaders
{
	class IShaderCompiler;

	struct ShaderSet
	{
		Interfaces::IShaderStage* VertexShader = nullptr;
		Interfaces::IShaderStage* PixelShader  = nullptr;
	};

	class ShaderCache
	{
	public:
		~ShaderCache( void );

		static ShaderCache* Instance( void );

		void InitCompiler( void );
		bool LoadCache( const std::string& _pCachePath );

		Interfaces::IEffect* GetEffect( const char* _pName );
		bool ReloadEffect(const char* _pName);

		const std::vector<Interfaces::IEffect*>* GetLoadedEffects(void) { return &m_vLoadedEffect; }

	private:
		ShaderCache( void );
		explicit ShaderCache( const std::string& _pShadersPath );

		Interfaces::IShaderCompiler* m_pShaderCompiler;
		std::vector<Interfaces::IEffect*>          m_vLoadedEffect;

		Effect* LoadShader(const char* _fileName);
		Interfaces::IShaderStage* LoadVertexShader(const char* _fileName);
		Interfaces::IShaderStage* LoadPixelShader(const char* _fileName);

		static void DumpShader( Interfaces::IShaderStage* _pShader );
	};
}

#endif // __ShaderCache_h__
