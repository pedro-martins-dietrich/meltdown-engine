#pragma once

#include <meltdown/model.hpp>

namespace mtd
{
	// Empty implementation of the Model class, used when no registered models were found.
	class EmptyModel : public Model
	{
		public:
			using Model::Model;

			virtual void start() override {}
			virtual void update(double deltaTime) override {}
	};
}
