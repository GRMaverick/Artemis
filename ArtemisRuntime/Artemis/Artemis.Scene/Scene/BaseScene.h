#pragma once

#include <vector>

namespace Artemis::Scene
{
	class BaseActor;

	class BaseScene
	{
	public:
		BaseScene();

	private:
		std::vector<BaseActor> Actors;
	};
}