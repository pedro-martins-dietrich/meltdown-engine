#include "RotatingModel.hpp"

#include "../Actions.hpp"
#include "../Events/InvertSpinEvent.hpp"

void RotatingModel::start()
{
	jumpCallbackHandle = mtd::EventManager::addCallback([&](const mtd::ActionStartEvent& event)
	{
		if(event.getAction() != Actions::Jump) return;
		isJumping = true;
	});
}

void RotatingModel::update(double deltaTime)
{
	transform.w.x -= deltaTime * transform.w.z;
	transform.w.z += deltaTime * transform.w.x;

	lightData.lightDirection.x -= 0.5f * deltaTime * lightData.lightDirection.z;
	lightData.lightDirection.z += 0.5f * deltaTime * lightData.lightDirection.x;
	lightData.ambientLightIntensity = 0.1f * lightData.lightDirection.z + 0.15f;
	mtd::EventManager::dispatch<mtd::UpdateDescriptorDataEvent>(0, 0, &lightData);

	if(isJumping)
		jumpAnimation(static_cast<float>(deltaTime));

	static uint32_t count = 0;
	count++;
	if(count >= 1000)
	{
		mtd::EventManager::dispatch<InvertSpinEvent>();
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
