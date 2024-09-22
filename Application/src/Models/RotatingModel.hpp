#pragma once

#include <meltdown/model.hpp>

class RotatingModel : public mtd::Model
{
	public:
		using Model::Model;

		virtual ~RotatingModel() override;

		virtual void start() override;
		virtual void update(double deltaTime) override;

	private:
		// ID of the callback for posterior deleting
		uint64_t callbackID;

		// Flag for jumping animation
		bool isJumping = false;
		// Current time for the jump animation (ranges from 0.0 to 1.0)
		float jumpT = 0.0f;

		// Updates the model position to create a jumping animation
		void jumpAnimation(float deltaTime);
};
