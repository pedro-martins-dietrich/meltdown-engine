#pragma once

#include <meltdown/model.hpp>

class SpinningModel : public mtd::Model
{
	public:
		using Model::Model;

		virtual void start() override;
		virtual void update(double deltaTime) override;

	private:
		float angularVelocity = 1.0f;
};
