#pragma once

#include <meltdown/model.hpp>

class RotatingModel : public mtd::Model
{
	public:
		using Model::Model;

		virtual void start() override;
		virtual void update(double deltaTime) override;
};
