#pragma once

#include <cstdio>
#include <stdlib.h>

#include "Interfaces/IEffect.h"

namespace Artemis::Renderer::Interfaces
{
	class IShaderStage;
}

namespace Artemis::Renderer::Shaders
{
	class Interfaces::IShaderStage;

	class Effect : public Interfaces::IEffect
	{
	public:
		Effect( Interfaces::IShaderStage* _pVertexShader, Interfaces::IShaderStage* _pPixelShader ) :
			m_pName{},
			m_pVertexShader( _pVertexShader ),
			m_pPixelShader( _pPixelShader )
		{
		}

		void SetName( const char* _pName ) { snprintf( m_pName, _countof( m_pName ), "%s", _pName ); }
		void SetPath( const char* _pPath ) { snprintf(m_pPath, _countof(m_pPath), "%s", _pPath); }

		virtual void SetVertexShader( Interfaces::IShaderStage* _pShader ) override { m_pVertexShader = _pShader; }
		virtual void SetPixelShader( Interfaces::IShaderStage* _pShader ) override { m_pPixelShader = _pShader; }

		virtual Interfaces::IShaderStage* GetVertexShader( void ) const override { return m_pVertexShader; }
		virtual Interfaces::IShaderStage* GetPixelShader( void ) const override { return m_pPixelShader; }

		virtual const char* GetName( void ) const override { return m_pName; }
		virtual const char* GetFilepath(void) const override { return m_pPath; }

	private:
		char m_pName[32];
		char m_pPath[128];

		Artemis::Renderer::Interfaces::IShaderStage* m_pVertexShader;
		Artemis::Renderer::Interfaces::IShaderStage* m_pPixelShader;
	};
}
