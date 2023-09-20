#include "ShaderDefines.h"

#include "ShaderCache.h"

#include <cassert>
#include <Windows.h>

#include "Effect.h"

#include "Loaders/CLParser.h"

#include "Interfaces/IShader.h"
#include "Factories/ShaderCompilerFactory.h"

#include "Constants/ConstantTable.h"

//#define DUMP_SHADERS

using namespace Artemis::Utilities;

namespace Artemis::Renderer::Shaders
{
	using namespace Interfaces;

	ShaderCache::ShaderCache( void ) :
		m_pShaderCompiler( nullptr )
	{
	}

	ShaderCache::ShaderCache( const std::string& _pShaderPaths )
	{
		LogInfo( "Loading ShaderCache:" );

		InitCompiler();

		LoadCache( _pShaderPaths.c_str() );
	}

	ShaderCache::~ShaderCache( void )
	{
	}

	ShaderCache* ShaderCache::Instance( void )
	{
		static ShaderCache cache;
		return &cache;
	}

	void ShaderCache::InitCompiler( void )
	{
		m_pShaderCompiler = Artemis::Renderer::Device::Factories::ShaderCompilerFactory::Construct(Artemis::Utilities::CLParser::Instance()->HasArgument("dxc"));
	}

	bool ShaderCache::LoadCache( const std::string& _strShadersPath )
	{
		if ( !m_pShaderCompiler )
		{
			InitCompiler();
			if ( !m_pShaderCompiler )
			{
				LogError( "\tInvalid Shader Compiler" );
				return false;
			}
		}

		WIN32_FIND_DATAA data = {};
		ZeroMemory( &data, sizeof(WIN32_FIND_DATAA) );

		const char*  pShaderPath = _strShadersPath.c_str();
		const HANDLE hFind       = FindFirstFileA( pShaderPath, &data );
		if ( hFind != INVALID_HANDLE_VALUE )
		{
			char* pDirectoryNoWildcard = new char[strlen( pShaderPath )];
			strncpy_s( pDirectoryNoWildcard, strlen( pShaderPath ), pShaderPath, strlen( pShaderPath ) - 1 );

			do
			{
				char* pFullFilepath = new char[strlen(pDirectoryNoWildcard) + strlen(data.cFileName) + 1];
				snprintf(pFullFilepath, strlen(pDirectoryNoWildcard) + strlen(data.cFileName) + 1, "%s%s", pDirectoryNoWildcard, data.cFileName);

				if ( strncmp( data.cFileName, ".", strlen( data.cFileName ) ) != 0 && 
					strncmp( data.cFileName, "..", strlen( data.cFileName ) ) != 0 && 
					(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY )
				{
					Effect* effect = LoadShader(pFullFilepath);
					effect->SetName(data.cFileName);
					effect->SetPath(pFullFilepath);

					m_vLoadedEffect.push_back( effect );
				}
			}
			while ( FindNextFileA( hFind, &data ) );

			delete[] pDirectoryNoWildcard;

			FindClose( hFind );
		}

		return true;
	}

	Effect* ShaderCache::LoadShader(const char* _fileName)
	{
		char pFullFullFileString[128];
		GetFullPathNameA(_fileName, ARRAYSIZE(pFullFullFileString), pFullFullFileString, nullptr);

		LogInfo("\t- %s", pFullFullFileString);

		IShaderStage* pVertexShader = LoadVertexShader(pFullFullFileString);
		IShaderStage* pPixelShader = LoadPixelShader(pFullFullFileString);

		return new Effect(pVertexShader, pPixelShader);
	}

	IShaderStage* ShaderCache::LoadVertexShader(const char* _fileName)
	{
		char* aError = nullptr;
		IShaderStage* pVertexShader = m_pShaderCompiler->Compile(_fileName, "MainVS", aError);
		if (!pVertexShader)
		{
			LogError("\t- %s failed to compile [Vertex Shader]", _fileName);
			return nullptr;
		}

		char pVShaderName[128] = { 0 };
		snprintf(pVShaderName, ARRAYSIZE(pVShaderName), "%s.vs", _fileName);
		pVertexShader->SetName(pVShaderName);
		m_pShaderCompiler->Reflect(pVertexShader);
		DumpShader(pVertexShader);

		ConstantTable::Instance()->CreateConstantBuffersEntries(pVertexShader->GetConstantParameters());

		return pVertexShader;
	}
	
	IShaderStage* ShaderCache::LoadPixelShader(const char* _fileName)
	{
		char* aError = nullptr;
		IShaderStage* pPixelShader = m_pShaderCompiler->Compile(_fileName, "MainPS", aError);
		if (!pPixelShader)
		{
			LogError("\t- %s failed to compile [Pixel Shader]", _fileName);
			return nullptr;
		}

		char pPShaderName[128] = { 0 };
		snprintf(pPShaderName, ARRAYSIZE(pPShaderName), "%s.ps", _fileName);
		pPixelShader->SetName(pPShaderName);
		m_pShaderCompiler->Reflect(pPixelShader);
		DumpShader(pPixelShader);

		ConstantTable::Instance()->CreateConstantBuffersEntries(pPixelShader->GetConstantParameters());

		return pPixelShader;
	}

	void ShaderCache::DumpShader( IShaderStage* _pShader )
	{
#if defined(_DEBUG) && defined(DUMP_SHADERS)
		const ShaderIOParameters& parameters = _pShader->GetShaderParameters();

		LogInfo("\t\tInputs: %u", parameters.NumberInputs);
		for (unsigned int parameter = 0; parameter < parameters.NumberInputs; ++parameter)
		{
			ShaderIOParameters::Parameter& p = parameters.Inputs[parameter];
			LogInfo("\t\t\tSemanticName: %s", p.SemanticName);
			LogInfo("\t\t\t\tSemanticIndex: %u", p.SemanticIndex);
			LogInfo("\t\t\t\tRegister: %u", p.Register);
			LogInfo("\t\t\t\tComponentType: %u", p.ComponentType);
			LogInfo("\t\t\t\tSystemValueType: %u", p.SystemValueType);
		}

		LogInfo("\t\tOutputs: %u", parameters.NumberOutputs);
		for (unsigned int parameter = 0; parameter < parameters.NumberOutputs; ++parameter)
		{
			ShaderIOParameters::Parameter& p = parameters.Outputs[parameter];
			LogInfo("\t\t\tSemanticName: %s", p.SemanticName);
			LogInfo("\t\t\t\tSemanticIndex: %u", p.SemanticIndex);
			LogInfo("\t\t\t\tRegister: %u", p.Register);
			LogInfo("\t\t\t\tComponentType: %u", p.ComponentType);
			LogInfo("\t\t\t\tSystemValueType: %u", p.SystemValueType);
		}
#endif
	}

	IEffect* ShaderCache::GetEffect( const char* _pName )
	{
		for ( size_t i = 0; i < m_vLoadedEffect.size(); ++i )
		{
			if ( strncmp( m_vLoadedEffect[i]->GetName(), _pName, strlen( _pName ) ) == 0 )
			{
				return m_vLoadedEffect[i];
			}
		}
		assert( "Effect does not exist" );
		return nullptr;
	}

	bool ShaderCache::ReloadEffect(const char* _pName)
	{
		for (size_t i = 0; i	< m_vLoadedEffect.size(); ++i)
		{
			//if (strncmp(m_vLoadedEffect[i]->GetName(), _pName, strlen(_pName)) == 0)
			if(strstr(m_vLoadedEffect[i]->GetName(), _pName))
			{
				IShaderStage* pVertexShader = LoadVertexShader(m_vLoadedEffect[i]->GetFilepath());
				IShaderStage* pPixelShader = LoadPixelShader(m_vLoadedEffect[i]->GetFilepath());

				m_vLoadedEffect[i]->SetVertexShader(pVertexShader);
				m_vLoadedEffect[i]->SetPixelShader(pPixelShader);

				return true;
			}
		}

		return false;
	}
}
