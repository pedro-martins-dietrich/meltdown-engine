#include <meltdown/model.hpp>

#include "EmptyModel.hpp"

mtd::ModelFactories mtd::ModelHandler::modelFactoryRegistry;

// Adds a model factory to the registry
mtd::ModelFactory mtd::ModelHandler::getModelFactory(const std::string& modelID)
{
	ModelFactories::const_iterator iterator = modelFactoryRegistry.find(modelID);
	if(iterator != modelFactoryRegistry.end())
		return iterator->second;

	return [](const Mat4x4& preTransform)
	{
		return std::make_unique<EmptyModel>(preTransform);
	};
}
