#include <pch.hpp>
#include "SceneLoader.hpp"

#include <nlohmann/json.hpp>

#include <Meltdown.hpp>

#include "../Vulkan/Mesh/DefaultMesh/DefaultMeshManager.hpp"
#include "../Vulkan/Mesh/Billboard/BillboardManager.hpp"
#include "../Vulkan/Mesh/MultiMaterial3D/MultiMaterial3DMeshManager.hpp"
#include "../Utils/FileHandler.hpp"
#include "../Utils/Logger.hpp"

static constexpr const char* sceneLoaderVersion = "0.1.6";

static void loadCamera(const nlohmann::json& cameraJson);
static void loadFramebuffer
(
	const nlohmann::json& framebufferJson,
	std::vector<mtd::FramebufferInfo>& framebufferInfos
);
static void loadPipeline
(
	const nlohmann::json& pipelineJson,
	std::vector<mtd::PipelineInfo>& pipelineInfos,
	std::vector<mtd::RenderPassInfo>& renderOrder
);
static void loadFramebufferPipeline
(
	const nlohmann::json& framebufferPipelineJson,
	std::vector<mtd::FramebufferPipelineInfo>& framebufferPipelineInfos,
	std::vector<mtd::RenderPassInfo>& renderOrder
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
	std::vector<FramebufferInfo>& framebufferInfos,
	std::vector<PipelineInfo>& pipelineInfos,
	std::vector<FramebufferPipelineInfo>& framebufferPipelineInfos,
	std::vector<RenderPassInfo>& renderOrder,
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

	loadCamera(sceneJson["camera"]);

	const nlohmann::json& framebuffersJson = sceneJson["framebuffers"];
	const nlohmann::json& pipelinesJson = sceneJson["pipelines"];
	const nlohmann::json& fbPipelinesJson = sceneJson["framebuffer-pipelines"];
	const nlohmann::json& meshesJson = sceneJson["meshes"];

	if(pipelinesJson.size() != meshesJson.size())
		LOG_ERROR("The number of pipelines and mesh managers should be the same.");
	
	framebufferInfos.reserve(framebuffersJson.size());
	pipelineInfos.reserve(pipelinesJson.size());
	framebufferInfos.reserve(fbPipelinesJson.size());
	renderOrder.reserve(framebuffersJson.size() + 1);
	meshManagers.reserve(meshesJson.size());

	for(int32_t i = 0; i < framebuffersJson.size(); i++)
	{
		loadFramebuffer(framebuffersJson[i], framebufferInfos);
		renderOrder.emplace_back(i);
	}
	renderOrder.emplace_back(-1);

	for(const nlohmann::json& pipelineJson: pipelinesJson)
		loadPipeline(pipelineJson, pipelineInfos, renderOrder);
	for(const nlohmann::json& fbPipelineJson: fbPipelinesJson)
		loadFramebufferPipeline(fbPipelineJson, framebufferPipelineInfos, renderOrder);

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

// Loads initial camera data for the scene
void loadCamera(const nlohmann::json& cameraJson)
{
	if(cameraJson["orthographic"])
	{
		mtd::EventManager::dispatch<mtd::SetOrthographicCameraEvent>
		(
			cameraJson["view-width"], cameraJson["far-plane"]
		);
	}
	else
	{
		mtd::EventManager::dispatch<mtd::SetPerspectiveCameraEvent>
		(
			cameraJson["fov"], cameraJson["near-plane"], cameraJson["far-plane"]
		);
	}

	mtd::CameraHandler::setPosition
	(
		mtd::Vec3{cameraJson["position"][0], cameraJson["position"][1], cameraJson["position"][2]}
	);
	mtd::CameraHandler::setOrientation(cameraJson["yaw"], cameraJson["pitch"]);
}

// Fetches the framebuffer info from the scene file
void loadFramebuffer
(
	const nlohmann::json& fbJson,
	std::vector<mtd::FramebufferInfo>& framebufferInfos
)
{
	framebufferInfos.emplace_back
	(
		mtd::FramebufferInfo
		{
			static_cast<mtd::FramebufferAttachments>(fbJson["attachments"]),
			static_cast<mtd::TextureSamplingFilterType>(fbJson["sampling-filter"]),
			{fbJson.value("resolution-ratio-horizontal", -1.0f), fbJson.value("resolution-ratio-vertical", -1.0f)},
			fbJson.value("fixed-horizontal-resolution", 1280U), fbJson.value("fixed-vertical-resolution", 720U)
		}
	);
}

// Fetches the pipeline info from the scene file
void loadPipeline
(
	const nlohmann::json& pipelineJson,
	std::vector<mtd::PipelineInfo>& pipelineInfos,
	std::vector<mtd::RenderPassInfo>& renderOrder
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

	int32_t targetFramebuffer = pipelineJson["target-framebuffer"];
	if(targetFramebuffer == -1)
		renderOrder.back().pipelineIndices.emplace_back(static_cast<uint32_t>(pipelineInfos.size()));
	else
		renderOrder[targetFramebuffer].pipelineIndices.emplace_back(static_cast<uint32_t>(pipelineInfos.size()));

	pipelineInfos.emplace_back
	(
		mtd::PipelineInfo
		{
			pipelineJson["name"],
			pipelineJson["vertex-shader"],
			pipelineJson["fragment-shader"],
			static_cast<mtd::MeshType>(pipelineJson["mesh-type"]),
			targetFramebuffer,
			std::move(descriptorInfos),
			static_cast<mtd::ShaderPrimitiveTopology>(pipelineJson["shader-primitive-topology"]),
			static_cast<mtd::ShaderFaceCulling>(pipelineJson["shader-face-culling"]),
			pipelineJson["transparency"],
			std::move(floatDataTypes),
			std::move(textureTypes)
		}
	);
}

// Fetches the framebuffer pipeline info from the scene file
void loadFramebufferPipeline
(
	const nlohmann::json& fbPipelineJson,
	std::vector<mtd::FramebufferPipelineInfo>& framebufferPipelineInfos,
	std::vector<mtd::RenderPassInfo>& renderOrder
)
{
	std::vector<mtd::DescriptorInfo> descriptorInfos;
	descriptorInfos.reserve(fbPipelineJson["descriptor-set-info"].size());
	for(const nlohmann::json& descriptorInfoJson: fbPipelineJson["descriptor-set-info"])
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

	const nlohmann::json& inputAttachmentsJson = fbPipelineJson["input-attachments"];
	std::vector<mtd::AttachmentIdentifier> inputAttachments(inputAttachmentsJson.size());
	for(uint32_t i = 0; i < inputAttachmentsJson.size(); i++)
	{
		inputAttachments[i].framebufferIndex = inputAttachmentsJson[i]["framebuffer-index"];
		inputAttachments[i].attachmentIndex = inputAttachmentsJson[i]["attachment-index"];
	}

	int32_t targetFramebuffer = fbPipelineJson["target-framebuffer"];
	if(targetFramebuffer == -1)
		renderOrder.back().framebufferPipelineIndex = static_cast<uint32_t>(framebufferPipelineInfos.size());
	else
		renderOrder[targetFramebuffer].framebufferPipelineIndex = static_cast<uint32_t>(framebufferPipelineInfos.size());

	framebufferPipelineInfos.emplace_back
	(
		mtd::FramebufferPipelineInfo
		{
			fbPipelineJson["name"],
			fbPipelineJson["vertex-shader"],
			fbPipelineJson["fragment-shader"],
			targetFramebuffer,
			std::move(descriptorInfos),
			std::move(inputAttachments),
			fbPipelineJson["dependencies"]
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
