#include "SpinningModel.hpp"

#include "../Events/CustomEventType.hpp"

uint32_t SpinningModel::count = 0;

SpinningModel::~SpinningModel()
{
	mtd::EventManager::removeCallback(CustomEventType::InvertSpin, callbackID);
}

void SpinningModel::start()
{
	count++;
	callbackID = mtd::EventManager::addCallback
	(
		CustomEventType::InvertSpin,
		[&](const mtd::Event& e)
		{
			angularVelocity = -angularVelocity;
		}
	);
}

void SpinningModel::update(double deltaTime)
{
	transform.rotateExtrinsic(angularVelocity * deltaTime, mtd::Vec3{0.0f, 1.0f, 0.0f});
}
