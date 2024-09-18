#pragma once

#include <meltdown/model.hpp>

class SpinningModel : public mtd::Model
{
	public:
		using Model::Model;

		virtual ~SpinningModel() override;

		virtual void start() override;
		virtual void update(double deltaTime) override;

	private:
		static uint32_t count;
		uint64_t callbackID;
		float angularVelocity = 1.0f;
};
