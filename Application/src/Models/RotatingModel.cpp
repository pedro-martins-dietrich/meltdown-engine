#include "RotatingModel.hpp"

#include "../Events/InvertSpinEvent.hpp"

void RotatingModel::start()
{
}

void RotatingModel::update(double deltaTime)
{
	transform[3][0] -= deltaTime * transform[3][2];
	transform[3][2] += deltaTime * transform[3][0];

	static uint32_t count = 0;
	count++;
	if(count >= 1000)
	{
		mtd::EventManager::dispatch(std::make_unique<InvertSpinEvent>());
		count = 0;
	}
}
