#include "RotatingModel.hpp"

#include "../Actions.hpp"
#include "../Events/InvertSpinEvent.hpp"

#include <iostream>

RotatingModel::~RotatingModel()
{
	mtd::EventManager::removeCallback(mtd::EventType::ActionStart, callbackID);
}

void RotatingModel::start()
{
	callbackID = mtd::EventManager::addCallback
	(
		mtd::EventType::ActionStart, [&](const mtd::Event& e)
		{
			const mtd::ActionStartEvent* ase = dynamic_cast<const mtd::ActionStartEvent*>(&e);
			if(!ase || ase->getAction() != Actions::Jump) return;

			isJumping = true;
		}
	);
}

void RotatingModel::update(double deltaTime)
{
	transform.w.x -= deltaTime * transform.w.z;
	transform.w.z += deltaTime * transform.w.x;

	if(isJumping)
		jumpAnimation(static_cast<float>(deltaTime));

	static uint32_t count = 0;
	count++;
	if(count >= 1000)
	{
		mtd::EventManager::dispatch(std::make_unique<InvertSpinEvent>());
		count = 0;
	}
}

// Updates the model position to create a jumping animation
void RotatingModel::jumpAnimation(float deltaTime)
{
	transform.w.y += 15.0f * (jumpT - 0.5f) * deltaTime;
	jumpT += deltaTime;
	if(jumpT > 1.0f)
	{
		isJumping = false;
		jumpT = 0.0f;
	}
}
