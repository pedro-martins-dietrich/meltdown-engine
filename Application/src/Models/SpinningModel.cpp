#include "SpinningModel.hpp"

#include "../Events/InvertSpinEvent.hpp"

void SpinningModel::start()
{
	invertSpinCallbackHandle = mtd::EventManager::addCallback([&](const InvertSpinEvent& event)
	{
		angularVelocity = -angularVelocity;
	});
}

void SpinningModel::update(double deltaTime)
{
	transform.rotateExtrinsic(angularVelocity * deltaTime, mtd::Vec3{0.0f, 1.0f, 0.0f});
}
