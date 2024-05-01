#include "SceneLoader.hpp"

#include <nlohmann/json.hpp>

#include "../Utils/FileHandler.hpp"
#include "../Utils/Logger.hpp"

#define SCENE_LOADER_VERSION "0.1.0"

static void getMesh(const nlohmann::json& meshJson, uint32_t id, std::vector<mtd::Mesh>& meshes);

// Loads the meshes from a Meltdown scene file
void mtd::SceneLoader::load(const char* fileName, std::vector<Mesh>& meshes)
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

	for(uint32_t i = 0; i < sceneJson["meshes"].size(); i++)
	{
		getMesh(sceneJson["meshes"][i], i, meshes);
	}

	LOG_INFO("Scene \"%s\" loaded.", fileName);
}

void getMesh(const nlohmann::json& meshJson, uint32_t id, std::vector<mtd::Mesh>& meshes)
{
	std::string file = meshJson["file"];
	std::vector<std::array<float, 16>> preTransforms = meshJson["pre-transforms"];

	meshes.emplace_back
	(
		file.c_str(),
		id,
		glm::mat4
		{
			preTransforms[0][0], preTransforms[0][1], preTransforms[0][2], preTransforms[0][3],
			preTransforms[0][4], preTransforms[0][5], preTransforms[0][6], preTransforms[0][7],
			preTransforms[0][8], preTransforms[0][9], preTransforms[0][10], preTransforms[0][11],
			preTransforms[0][12], preTransforms[0][13], preTransforms[0][14], preTransforms[0][15]
		}
	);

	for(uint32_t i = 1; i < preTransforms.size(); i++)
	{
		meshes.back().addInstance(glm::mat4
		{
			preTransforms[i][0], preTransforms[i][1], preTransforms[i][2], preTransforms[i][3],
			preTransforms[i][4], preTransforms[i][5], preTransforms[i][6], preTransforms[i][7],
			preTransforms[i][8], preTransforms[i][9], preTransforms[i][10], preTransforms[i][11],
			preTransforms[i][12], preTransforms[i][13], preTransforms[i][14], preTransforms[i][15]
		});
	}
}
