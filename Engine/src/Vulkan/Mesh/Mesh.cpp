#include "Mesh.hpp"

#include "../../Utils/Logger.hpp"

mtd::Mesh::Mesh(uint32_t index, const char* modelID, const Mat4x4& preTransform)
	: meshIndex{index},
	modelID{modelID},
	modelFactory{ModelHandler::getModelFactory(modelID)},
	models{},
	instanceLumpOffset{0},
	pInstanceLump{nullptr}
{
	models.emplace_back(modelFactory(preTransform));
}

mtd::Mesh::Mesh
(
	uint32_t index,
	const std::string& modelID,
	std::vector<std::unique_ptr<Model>>&& models,
	size_t instanceLumpOffset,
	std::vector<Mat4x4>* pInstanceLump
) : meshIndex{index},
	modelID{modelID},
	modelFactory{ModelHandler::getModelFactory(modelID)},
	models{std::move(models)},
	instanceLumpOffset{instanceLumpOffset},
	pInstanceLump{pInstanceLump}
{
}

// Runs once at the beginning of the scene for all instances
void mtd::Mesh::start()
{
	for(uint32_t instanceIndex = 0; instanceIndex < models.size(); instanceIndex++)
	{
		models[instanceIndex]->start();
		memcpy
		(
			&(*pInstanceLump)[instanceLumpOffset + instanceIndex],
			models[instanceIndex]->getTransformPointer(),
			sizeof(Mat4x4)
		);
	}
}

// Updates all instances
void mtd::Mesh::update(double deltaTime)
{
	for(uint32_t instanceIndex = 0; instanceIndex < models.size(); instanceIndex++)
	{
		models[instanceIndex]->update(deltaTime);
		memcpy
		(
			&(*pInstanceLump)[instanceLumpOffset + instanceIndex],
			models[instanceIndex]->getTransformPointer(),
			sizeof(Mat4x4)
		);
	}
}

// Sets a reference to the instance lump to update the instances data
void mtd::Mesh::setInstancesLump(std::vector<Mat4x4>* instanceLumpPointer, size_t offset)
{
	pInstanceLump = instanceLumpPointer;
	instanceLumpOffset = offset;

	pInstanceLump->reserve(models.size());
	for(const std::unique_ptr<Model>& model: models)
		pInstanceLump->push_back(*(model->getTransformPointer()));
}

// Adds a new instance
void mtd::Mesh::addInstance(const Mat4x4& preTransform)
{
	models.emplace_back(modelFactory(preTransform));
}
