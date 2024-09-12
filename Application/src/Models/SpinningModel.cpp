#include "SpinningModel.hpp"

#include <cmath>
#include <iostream>

void SpinningModel::start()
{
}

void SpinningModel::update(double deltaTime)
{
	transform.rotateExtrinsic(angularVelocity * deltaTime, mtd::Vec3{0.0f, 1.0f, 0.0f});
}
