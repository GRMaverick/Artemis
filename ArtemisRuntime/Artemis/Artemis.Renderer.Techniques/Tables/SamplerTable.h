#pragma once

#include <map>

#include "Interfaces/ISamplerState.h"

namespace Artemis::Renderer::Techniques::Tables
{
	class SamplerTable
	{
	public:
		~SamplerTable(void);
		
		static SamplerTable* Instance(void);

		bool CreateSamplerState(const char* _name, Interfaces::SamplerStateFilter _eFilter, Interfaces::SamplerStateWrapMode _eWrap, Interfaces::SamplerStateComparisonFunction _eCompFunc, Interfaces::IGraphicsDevice* _pDevice);
		Interfaces::ISamplerState* GetSamplerState(const char* _name);

	private:
		SamplerTable(void);
		std::map<unsigned int, Interfaces::ISamplerState*> m_mapSamplerStates;
	};
}