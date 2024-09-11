#include "RotatingModel.hpp"

#include <iostream>

void RotatingModel::start()
{
}

void RotatingModel::update(double deltaTime)
{
	transform[3][0] -= deltaTime * transform[3][2];
	transform[3][2] += deltaTime * transform[3][0];
}
