#include <pch.hpp>
#include "SceneLoader.hpp"

#include <nlohmann/json.hpp>

#include "../Vulkan/Mesh/DefaultMesh/DefaultMeshManager.hpp"
#include "../Vulkan/Mesh/Billboard/BillboardManager.hpp"
#include "../Utils/FileHandler.hpp"
#include "../Utils/Logger.hpp"

#define SCENE_LOADER_VERSION "0.1.2"

static void getDefaultMesh
(
	const mtd::Device& device,
	const nlohmann::json& meshJson,
	uint32_t index,
	mtd::MeshManager* pMeshManager
);
static void getBillboard
(
	const mtd::Device& device,
	const nlohmann::json& billboardJson,
	uint32_t index,
	mtd::MeshManager* pMeshManager
);

// Loads the meshes from a Meltdown scene file
void mtd::SceneLoader::load
(
	const Device& device,
	const char* fileName,
	std::unordered_map<MeshType, std::unique_ptr<MeshManager>>& meshManagers
)
{
	std::string scenePath{MTD_RESOURCES_PATH};
	scenePath.append("scenes/");
	scenePath.append(fileName);

	nlohmann::json sceneJson;
	if(!FileHandler::readJSON(scenePath.c_str(), sceneJson)) return;

	std::string fileVersion = sceneJson["scene-loader-version"];
	if(fileVersion.compare(SCENE_LOADER_VERSION))
	{
		LOG_ERROR
		(
			"Scene JSON file (\"%s\") version is incompatible with the scene loader.",
			fileName
		);
		return;
	}

	for(uint32_t i = 0; i < sceneJson["default-meshes"].size(); i++)
		getDefaultMesh(device, sceneJson["default-meshes"][i], i, meshManagers.at(MeshType::Default3D).get());
	for(uint32_t i = 0; i < sceneJson["billboards"].size(); i++)
		getBillboard(device, sceneJson["billboards"][i], i, meshManagers.at(MeshType::Billboard).get());

	LOG_INFO("Scene \"%s\" loaded.", fileName);
}

// Fetches default meshes from scene file
void getDefaultMesh
(
	const mtd::Device& device,
	const nlohmann::json& meshJson,
	uint32_t index,
	mtd::MeshManager* pMeshManager
)
{
	std::string file = meshJson["file"];
	std::string id = meshJson.value("model-id", "");
	const std::vector<std::array<float, 16>>& preTransforms = meshJson["pre-transforms"];
	const std::vector<mtd::Mat4x4>* pPreTransforms = reinterpret_cast<const std::vector<mtd::Mat4x4>*>(&preTransforms);

	std::vector<mtd::DefaultMesh>& meshes = dynamic_cast<mtd::DefaultMeshManager*>(pMeshManager)->getMeshes();
	meshes.emplace_back(device, index, id.c_str(), file.c_str(), *pPreTransforms);
}

// Fetches billboards from scene file
void getBillboard
(
	const mtd::Device& device,
	const nlohmann::json& billboardJson,
	uint32_t index,
	mtd::MeshManager* pMeshManager
)
{
	std::string texturePath = billboardJson["texture"];
	std::string id = billboardJson.value("model-id", "");
	const std::vector<std::array<float, 16>>& preTransforms = billboardJson["pre-transforms"];
	const std::vector<mtd::Mat4x4>* pPreTransforms = reinterpret_cast<const std::vector<mtd::Mat4x4>*>(&preTransforms);

	std::vector<mtd::Billboard>& billboards = dynamic_cast<mtd::BillboardManager*>(pMeshManager)->getMeshes();
	billboards.emplace_back(device, index, id.c_str(), texturePath.c_str(), *pPreTransforms);
}
