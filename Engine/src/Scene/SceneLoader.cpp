#include <pch.hpp>
#include "SceneLoader.hpp"

#include <nlohmann/json.hpp>

#include "../Vulkan/Mesh/DefaultMesh/DefaultMeshManager.hpp"
#include "../Vulkan/Mesh/MultiMaterial3D/MultiMaterial3DMeshManager.hpp"
#include "../Vulkan/Mesh/Billboard/BillboardManager.hpp"
#include "../Utils/FileHandler.hpp"
#include "../Utils/Logger.hpp"

static constexpr const char* sceneLoaderVersion = "0.1.4";

static void loadPipeline
(
	const mtd::Device& device,
	const nlohmann::json& pipelinesJson,
	std::vector<mtd::PipelineInfo>& pipelineInfos
);
static void loadDefaultMeshes
(
	const mtd::Device& device,
	const nlohmann::json& meshListJson,
	const mtd::PipelineInfo& pipelineInfo,
	std::vector<std::unique_ptr<mtd::MeshManager>>& meshManagers
);
static void loadMultiMaterial3DMeshes
(
	const mtd::Device& device,
	const nlohmann::json& meshListJson,
	const mtd::PipelineInfo& pipelineInfo,
	std::vector<std::unique_ptr<mtd::MeshManager>>& meshManagers
);
static void loadBillboards
(
	const mtd::Device& device,
	const nlohmann::json& billboardListJson,
	const mtd::PipelineInfo& pipelineInfo,
	std::vector<std::unique_ptr<mtd::MeshManager>>& meshManagers
);

// Loads the meshes from a Meltdown scene file
void mtd::SceneLoader::load
(
	const Device& device,
	const char* fileName,
	std::vector<PipelineInfo>& pipelineInfos,
	std::vector<std::unique_ptr<MeshManager>>& meshManagers
)
{
	std::string scenePath{MTD_RESOURCES_PATH};
	scenePath.append("scenes/");
	scenePath.append(fileName);

	nlohmann::json sceneJson;
	if(!FileHandler::readJSON(scenePath.c_str(), sceneJson)) return;

	std::string fileVersion = sceneJson["scene-loader-version"];
	if(fileVersion.compare(sceneLoaderVersion))
	{
		LOG_ERROR("Scene JSON file (\"%s\") version is incompatible with the scene loader.", fileName);
		return;
	}

	const nlohmann::json& meshesJson = sceneJson["meshes"];

	assert
	(
		sceneJson["pipelines"].size() == meshesJson.size() &&
		"The number of pipelines and mesh managers should be the same."
	);

	pipelineInfos.reserve(sceneJson["pipelines"].size());
	meshManagers.reserve(meshesJson.size());

	for(const nlohmann::json pipelineJson: sceneJson["pipelines"])
		loadPipeline(device, pipelineJson, pipelineInfos);

	for(uint32_t i = 0; i < meshesJson.size(); i++)
	{
		switch(pipelineInfos[i].associatedMeshType)
		{
			case MeshType::Default3D:
				loadDefaultMeshes(device, meshesJson[i], pipelineInfos[i], meshManagers);
				break;
			case MeshType::MultiMaterial3D:
				loadMultiMaterial3DMeshes(device, meshesJson[i], pipelineInfos[i], meshManagers);
				break;
			case MeshType::Billboard:
				loadBillboards(device, meshesJson[i], pipelineInfos[i], meshManagers);
				break;
			default:
				LOG_ERROR("Unknown mesh type (%d) for mesh at index %d.", pipelineInfos[i].associatedMeshType, i);
		}
	}

	LOG_INFO("Scene \"%s\" loaded.", fileName);
}

// Fetches the pipeline info from the scene file
void loadPipeline
(
	const mtd::Device& device,
	const nlohmann::json& pipelineJson,
	std::vector<mtd::PipelineInfo>& pipelineInfos
)
{
	std::vector<mtd::DescriptorInfo> descriptorInfos;
	descriptorInfos.reserve(pipelineJson["descriptor-set-info"].size());
	for(const nlohmann::json& descriptorInfoJson: pipelineJson["descriptor-set-info"])
	{
		descriptorInfos.emplace_back
		(
			mtd::DescriptorInfo
			{
				static_cast<mtd::DescriptorType>(descriptorInfoJson["descriptor-type"]),
				static_cast<mtd::ShaderStage>(descriptorInfoJson["shader-stage"]),
				descriptorInfoJson["total-descriptor-size"],
				descriptorInfoJson["descriptor-count"]
			}
		);
	}

	std::vector<mtd::MaterialFloatDataType> floatDataTypes;
	floatDataTypes.reserve(pipelineJson["material-float-data-types"].size());
	for(const nlohmann::json& floatDataType: pipelineJson["material-float-data-types"])
		floatDataTypes.emplace_back(static_cast<mtd::MaterialFloatDataType>(floatDataType));

	std::vector<mtd::MaterialTextureType> textureTypes;
	textureTypes.reserve(pipelineJson["material-texture-types"].size());
	for(const nlohmann::json& textureType: pipelineJson["material-texture-types"])
		textureTypes.emplace_back(static_cast<mtd::MaterialTextureType>(textureType));

	pipelineInfos.emplace_back(
		mtd::PipelineInfo
		{
			pipelineJson["name"],
			pipelineJson["vertex-shader"],
			pipelineJson["fragment-shader"],
			static_cast<mtd::MeshType>(pipelineJson["mesh-type"]),
			std::move(descriptorInfos),
			static_cast<mtd::ShaderPrimitiveTopology>(pipelineJson["shader-primitive-topology"]),
			static_cast<mtd::ShaderFaceCulling>(pipelineJson["shader-face-culling"]),
			pipelineJson["transparency"],
			std::move(floatDataTypes),
			std::move(textureTypes)
		}
	);
}

// Fetches default meshes from scene file
void loadDefaultMeshes
(
	const mtd::Device& device,
	const nlohmann::json& meshListJson,
	const mtd::PipelineInfo& pipelineInfo,
	std::vector<std::unique_ptr<mtd::MeshManager>>& meshManagers
)
{
	meshManagers.emplace_back(std::make_unique<mtd::DefaultMeshManager>(device));
	mtd::MaterialInfo materialInfo{pipelineInfo.materialFloatDataTypes, pipelineInfo.materialTextureTypes};

	for(uint32_t i = 0; i < meshListJson.size(); i++)
	{
		const std::string& id = meshListJson[i].value("model-id", "");
		const std::string& file = meshListJson[i]["file"];

		const std::vector<std::array<float, 16>>& preTransforms = meshListJson[i]["pre-transforms"];
		const std::vector<mtd::Mat4x4>* pPreTransforms =
			reinterpret_cast<const std::vector<mtd::Mat4x4>*>(&preTransforms);

		std::vector<mtd::DefaultMesh>& meshes =
			dynamic_cast<mtd::DefaultMeshManager*>(meshManagers.back().get())->getMeshes();
		meshes.emplace_back(device, i, id.c_str(), file.c_str(), materialInfo, *pPreTransforms);
	}
}

// Fetches default meshes from scene file
void loadMultiMaterial3DMeshes
(
	const mtd::Device& device,
	const nlohmann::json& meshListJson,
	const mtd::PipelineInfo& pipelineInfo,
	std::vector<std::unique_ptr<mtd::MeshManager>>& meshManagers
)
{
	meshManagers.emplace_back(std::make_unique<mtd::MultiMaterial3DMeshManager>(device));
	mtd::MaterialInfo materialInfo{pipelineInfo.materialFloatDataTypes, pipelineInfo.materialTextureTypes};

	for(uint32_t i = 0; i < meshListJson.size(); i++)
	{
		const std::string& id = meshListJson[i].value("model-id", "");
		const std::string& file = meshListJson[i]["file"];

		const std::vector<std::array<float, 16>>& preTransforms = meshListJson[i]["pre-transforms"];
		const std::vector<mtd::Mat4x4>* pPreTransforms =
			reinterpret_cast<const std::vector<mtd::Mat4x4>*>(&preTransforms);

		std::vector<mtd::MultiMaterial3DMesh>& meshes =
			dynamic_cast<mtd::MultiMaterial3DMeshManager*>(meshManagers.back().get())->getMeshes();
		meshes.emplace_back(device, i, id.c_str(), file.c_str(), materialInfo, *pPreTransforms);
	}
}

// Fetches billboards from scene file
void loadBillboards
(
	const mtd::Device& device,
	const nlohmann::json& billboardListJson,
	const mtd::PipelineInfo& pipelineInfo,
	std::vector<std::unique_ptr<mtd::MeshManager>>& meshManagers
)
{
	meshManagers.emplace_back(std::make_unique<mtd::BillboardManager>(device));
	mtd::MaterialInfo materialInfo{pipelineInfo.materialFloatDataTypes, pipelineInfo.materialTextureTypes};

	for(uint32_t i = 0; i < billboardListJson.size(); i++)
	{
		const std::string& id = billboardListJson[i].value("model-id", "");
		const std::string& textureFile = billboardListJson[i]["texture"];
		std::string texturePath = MTD_RESOURCES_PATH + textureFile;

		const std::vector<std::array<float, 16>>& preTransforms = billboardListJson[i]["pre-transforms"];
		const std::vector<mtd::Mat4x4>* pPreTransforms =
			reinterpret_cast<const std::vector<mtd::Mat4x4>*>(&preTransforms);

		std::vector<mtd::Billboard>& billboards =
			dynamic_cast<mtd::BillboardManager*>(meshManagers.back().get())->getMeshes();
		billboards.emplace_back(device, i, id.c_str(), texturePath.c_str(), materialInfo, *pPreTransforms);
	}
}
