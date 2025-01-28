#pragma once

#include <meltdown/event.hpp>
#include <meltdown/model.hpp>

class SpinningModel : public mtd::Model
{
	public:
		using Model::Model;

		virtual void start() override;
		virtual void update(double deltaTime) override;

	private:
		mtd::EventCallbackHandle invertSpinCallbackHandle;
		float angularVelocity = 1.0f;
};
