#include "SamplerTable.h"

#include "Hashing/Hashing.h"
#include "Interfaces/IGraphicsDevice.h"

using namespace Artemis::Utilities;
using namespace Artemis::Renderer::Interfaces;

namespace Artemis::Renderer::Techniques::Tables
{
	SamplerTable::SamplerTable(void)
	{

	}

	SamplerTable::~SamplerTable(void)
	{

	}

	SamplerTable* SamplerTable::Instance(void)
	{
		static SamplerTable table;
		return &table;
	}

	bool SamplerTable::CreateSamplerState(const char* _name, Interfaces::SamplerStateFilter _eFilter, Interfaces::SamplerStateWrapMode _eWrap, Interfaces::SamplerStateComparisonFunction _eCompFunc, IGraphicsDevice* _pDevice)
	{
		ISamplerState* pState = _pDevice->CreateSamplerState(_eFilter, _eWrap, _eCompFunc);
		m_mapSamplerStates.emplace(SimpleHash(_name, strlen(_name)), pState);

		return true;
	}

	Interfaces::ISamplerState* SamplerTable::GetSamplerState(const char* _name)
	{
		unsigned long long hash = SimpleHash(_name, strlen(_name));
		if (m_mapSamplerStates.contains(hash))
		{
			return m_mapSamplerStates[hash];
		}
	}
}