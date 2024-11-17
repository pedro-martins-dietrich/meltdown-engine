#include <pch.hpp>
#include <meltdown/model.hpp>

#include "EmptyModel.hpp"

using ModelInstanceMap = std::unordered_map<uint64_t, mtd::Model*>;
static ModelInstanceMap modelInstanceRegistry;

static uint64_t nextModelInstanceID = 0;

mtd::ModelFactories mtd::ModelHandler::modelFactoryRegistry;

// Model
mtd::Model::Model(const char* modelID, const Mat4x4& preTransform)
	: transform{preTransform}, instanceID{nextModelInstanceID++}
{
	modelInstanceRegistry[instanceID] = this;
}

mtd::Model::~Model()
{
	modelInstanceRegistry.erase(instanceID);
}

const mtd::Mat4x4& mtd::Model::getTransform() const
{
	return transform;
}

mtd::Mat4x4& mtd::Model::getTransform()
{
	return transform;
}

uint64_t mtd::Model::getInstanceID() const
{
	return instanceID;
}

// Model Handler
mtd::ModelFactory mtd::ModelHandler::getModelFactory(const std::string& modelID)
{
	ModelFactories::const_iterator iterator = modelFactoryRegistry.find(modelID);
	if(iterator != modelFactoryRegistry.end())
		return iterator->second;

	return [](const Mat4x4& preTransform)
	{
		return std::make_unique<EmptyModel>("", preTransform);
	};
}

mtd::Model* mtd::ModelHandler::getModelInstanceByID(uint64_t instanceID)
{
	ModelInstanceMap::const_iterator iterator = modelInstanceRegistry.find(instanceID);
	return (iterator != modelInstanceRegistry.end()) ? iterator->second : nullptr;
}

const std::unordered_map<uint64_t, mtd::Model*>& mtd::ModelHandler::getModelInstancesMap()
{
	return modelInstanceRegistry;
}
