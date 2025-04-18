#include <pch.hpp>
#include "SceneLoader.hpp"

#include <nlohmann/json.hpp>

#include <Meltdown.hpp>

#include "../Vulkan/Mesh/DefaultMesh/DefaultMeshManager.hpp"
#include "../Vulkan/Mesh/Billboard/BillboardManager.hpp"
#include "../Vulkan/Mesh/MultiMaterial3D/MultiMaterial3DMeshManager.hpp"
#include "../Vulkan/Mesh/RayTracingMesh/RayTracingMeshManager.hpp"
#include "../Utils/FileHandler.hpp"
#include "../Utils/Logger.hpp"

static constexpr const char* sceneLoaderVersion = "0.1.8";

static void loadCamera(const nlohmann::json& cameraJson);
static void loadFramebuffer
(
	const nlohmann::json& framebufferJson,
	std::vector<mtd::FramebufferInfo>& framebufferInfos
);
static void loadPipeline
(
	const nlohmann::json& pipelineJson,
	std::vector<mtd::GraphicsPipelineInfo>& graphicsPipelineInfos,
	std::vector<mtd::RenderPassInfo>& renderOrder
);
static void loadRayTracingPipeline
(
	const nlohmann::json& rayTracingPipelineJson,
	std::vector<mtd::RayTracingPipelineInfo>& rayTracingPipelineInfos
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
	const mtd::GraphicsPipelineInfo& graphicsPipelineInfo,
	std::vector<std::unique_ptr<mtd::MeshManager>>& meshManagers
);
static void loadMultiMaterial3DMeshes
(
	const mtd::Device& device,
	const nlohmann::json& meshListJson,
	const mtd::GraphicsPipelineInfo& graphicsPipelineInfo,
	std::vector<std::unique_ptr<mtd::MeshManager>>& meshManagers
);
static void loadBillboards
(
	const mtd::Device& device,
	const nlohmann::json& billboardListJson,
	const mtd::GraphicsPipelineInfo& graphicsPipelineInfo,
	std::vector<std::unique_ptr<mtd::MeshManager>>& meshManagers
);
static void loadRayTracingMeshes
(
	const mtd::Device& device,
	const nlohmann::json& rtMeshListJson,
	const mtd::RayTracingPipelineInfo& rtPipelineInfo,
	std::vector<std::unique_ptr<mtd::MeshManager>>& meshManagers
);

// Loads the meshes from a Meltdown scene file
void mtd::SceneLoader::load
(
	const Device& device,
	const char* fileName,
	std::vector<FramebufferInfo>& framebufferInfos,
	std::vector<GraphicsPipelineInfo>& graphicsPipelineInfos,
	std::vector<FramebufferPipelineInfo>& framebufferPipelineInfos,
	std::vector<RayTracingPipelineInfo>& rayTracingPipelineInfos,
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
	const nlohmann::json& rasterPipelinesJson = sceneJson["rasterization-pipelines"];
	const nlohmann::json& fbPipelinesJson = sceneJson["framebuffer-pipelines"];
	const nlohmann::json& rtPipelinesJson = sceneJson["ray-tracing-pipelines"];
	const nlohmann::json& meshesJson = sceneJson["meshes"];

	if((rasterPipelinesJson.size() + rtPipelinesJson.size()) != meshesJson.size())
		LOG_ERROR("The number of pipelines and mesh managers should be the same.");
	
	framebufferInfos.reserve(framebuffersJson.size());
	graphicsPipelineInfos.reserve(rasterPipelinesJson.size());
	rayTracingPipelineInfos.reserve(rtPipelinesJson.size());
	framebufferPipelineInfos.reserve(fbPipelinesJson.size());
	renderOrder.reserve(framebuffersJson.size() + 1);
	meshManagers.reserve(meshesJson.size());

	for(int32_t i = 0; i < framebuffersJson.size(); i++)
	{
		loadFramebuffer(framebuffersJson[i], framebufferInfos);
		renderOrder.emplace_back(i);
	}
	renderOrder.emplace_back(-1);

	for(const nlohmann::json& rasterPipelineJson: rasterPipelinesJson)
		loadPipeline(rasterPipelineJson, graphicsPipelineInfos, renderOrder);
	for(const nlohmann::json& rtPipelineJson: rtPipelinesJson)
		loadRayTracingPipeline(rtPipelineJson, rayTracingPipelineInfos);
	for(const nlohmann::json& fbPipelineJson: fbPipelinesJson)
		loadFramebufferPipeline(fbPipelineJson, framebufferPipelineInfos, renderOrder);

	for(uint32_t i = 0; i < meshesJson.size(); i++)
	{
		if(i >= graphicsPipelineInfos.size())
		{
			uint32_t rtPipelineIndex = i - rasterPipelinesJson.size();
			loadRayTracingMeshes(device, meshesJson[i], rayTracingPipelineInfos[rtPipelineIndex], meshManagers);
			continue;
		}

		switch(graphicsPipelineInfos[i].associatedMeshType)
		{
			case MeshType::Default3D:
				loadDefaultMeshes(device, meshesJson[i], graphicsPipelineInfos[i], meshManagers);
				break;
			case MeshType::MultiMaterial3D:
				loadMultiMaterial3DMeshes(device, meshesJson[i], graphicsPipelineInfos[i], meshManagers);
				break;
			case MeshType::Billboard:
				loadBillboards(device, meshesJson[i], graphicsPipelineInfos[i], meshManagers);
				break;
			default:
				LOG_ERROR("Unknown type (%d) for mesh at index %d.", graphicsPipelineInfos[i].associatedMeshType, i);
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

// Fetches the pipeline infos from the scene file
void loadPipeline
(
	const nlohmann::json& graphicsPipelineJson,
	std::vector<mtd::GraphicsPipelineInfo>& graphicsPipelineInfos,
	std::vector<mtd::RenderPassInfo>& renderOrder
)
{
	std::vector<mtd::DescriptorInfo> descriptorInfos;
	descriptorInfos.reserve(graphicsPipelineJson["descriptor-set-info"].size());
	for(const nlohmann::json& descriptorInfoJson: graphicsPipelineJson["descriptor-set-info"])
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
	floatDataTypes.reserve(graphicsPipelineJson["material-float-data-types"].size());
	for(const nlohmann::json& floatDataType: graphicsPipelineJson["material-float-data-types"])
		floatDataTypes.emplace_back(static_cast<mtd::MaterialFloatDataType>(floatDataType));

	std::vector<mtd::MaterialTextureType> textureTypes;
	textureTypes.reserve(graphicsPipelineJson["material-texture-types"].size());
	for(const nlohmann::json& textureType: graphicsPipelineJson["material-texture-types"])
		textureTypes.emplace_back(static_cast<mtd::MaterialTextureType>(textureType));

	int32_t targetFramebuffer = graphicsPipelineJson["target-framebuffer"];
	if(targetFramebuffer == -1)
		renderOrder.back().pipelineIndices.emplace_back(static_cast<uint32_t>(graphicsPipelineInfos.size()));
	else
		renderOrder[targetFramebuffer].pipelineIndices.emplace_back(static_cast<uint32_t>(graphicsPipelineInfos.size()));

	graphicsPipelineInfos.emplace_back
	(
		mtd::GraphicsPipelineInfo
		{
			{graphicsPipelineJson["name"], std::move(descriptorInfos)},
			graphicsPipelineJson["vertex-shader"],
			graphicsPipelineJson["fragment-shader"],
			static_cast<mtd::MeshType>(graphicsPipelineJson["mesh-type"]),
			targetFramebuffer,
			static_cast<mtd::ShaderPrimitiveTopology>(graphicsPipelineJson["shader-primitive-topology"]),
			static_cast<mtd::ShaderFaceCulling>(graphicsPipelineJson["shader-face-culling"]),
			graphicsPipelineJson["transparency"],
			std::move(floatDataTypes),
			std::move(textureTypes)
		}
	);
}

// Fetches the ray tracing pipeline infos from the scene file
void loadRayTracingPipeline
(
	const nlohmann::json& rtPipelineJson,
	std::vector<mtd::RayTracingPipelineInfo>& rayTracingPipelineInfos
)
{
	std::vector<mtd::DescriptorInfo> descriptorInfos;
	descriptorInfos.reserve(rtPipelineJson["descriptor-set-info"].size());
	for(const nlohmann::json& descriptorInfoJson: rtPipelineJson["descriptor-set-info"])
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
	floatDataTypes.reserve(rtPipelineJson["material-float-data-types"].size());
	for(const nlohmann::json& floatDataType: rtPipelineJson["material-float-data-types"])
		floatDataTypes.emplace_back(static_cast<mtd::MaterialFloatDataType>(floatDataType));

	std::vector<mtd::MaterialTextureType> textureTypes;
	textureTypes.reserve(rtPipelineJson["material-texture-types"].size());
	for(const nlohmann::json& textureType: rtPipelineJson["material-texture-types"])
		textureTypes.emplace_back(static_cast<mtd::MaterialTextureType>(textureType));

	rayTracingPipelineInfos.emplace_back
	(
		mtd::RayTracingPipelineInfo
		{
			{rtPipelineJson["name"], std::move(descriptorInfos)},
			rtPipelineJson["ray-gen-shader"],
			rtPipelineJson["miss-shader"],
			{
				rtPipelineJson.value("resolution-ratio-horizontal", -1.0f),
				rtPipelineJson.value("resolution-ratio-vertical", -1.0f)
			},
			rtPipelineJson.value("fixed-horizontal-resolution", 1280U),
			rtPipelineJson.value("fixed-vertical-resolution", 720U),
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
			{fbPipelineJson["name"], std::move(descriptorInfos)},
			fbPipelineJson["vertex-shader"],
			fbPipelineJson["fragment-shader"],
			targetFramebuffer,
			std::move(inputAttachments),
			fbPipelineJson["ray-tracing-storage-images"],
			fbPipelineJson["dependencies"]
		}
	);
}

// Fetches default meshes from scene file
void loadDefaultMeshes
(
	const mtd::Device& device,
	const nlohmann::json& meshListJson,
	const mtd::GraphicsPipelineInfo& graphicsPipelineInfo,
	std::vector<std::unique_ptr<mtd::MeshManager>>& meshManagers
)
{
	meshManagers.emplace_back(std::make_unique<mtd::DefaultMeshManager>(device));
	mtd::MaterialInfo materialInfo
	{
		graphicsPipelineInfo.materialFloatDataTypes, graphicsPipelineInfo.materialTextureTypes
	};

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
	const mtd::GraphicsPipelineInfo& graphicsPipelineInfo,
	std::vector<std::unique_ptr<mtd::MeshManager>>& meshManagers
)
{
	meshManagers.emplace_back(std::make_unique<mtd::MultiMaterial3DMeshManager>(device));
	mtd::MaterialInfo materialInfo
	{
		graphicsPipelineInfo.materialFloatDataTypes, graphicsPipelineInfo.materialTextureTypes
	};

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
	const mtd::GraphicsPipelineInfo& graphicsPipelineInfo,
	std::vector<std::unique_ptr<mtd::MeshManager>>& meshManagers
)
{
	meshManagers.emplace_back(std::make_unique<mtd::BillboardManager>(device));
	mtd::MaterialInfo materialInfo
	{
		graphicsPipelineInfo.materialFloatDataTypes, graphicsPipelineInfo.materialTextureTypes
	};

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

// Fetches ray tracing meshes from the scene file
void loadRayTracingMeshes
(
	const mtd::Device& device,
	const nlohmann::json& rtMeshListJson,
	const mtd::RayTracingPipelineInfo& rtPipelineInfo,
	std::vector<std::unique_ptr<mtd::MeshManager>>& meshManagers
)
{
	mtd::MaterialInfo materialInfo{rtPipelineInfo.materialFloatDataTypes, rtPipelineInfo.materialTextureTypes};
	meshManagers.emplace_back(std::make_unique<mtd::RayTracingMeshManager>(device, materialInfo));

	mtd::RayTracingMeshManager* pMeshManager = dynamic_cast<mtd::RayTracingMeshManager*>(meshManagers.back().get());

	for(uint32_t i = 0; i < rtMeshListJson.size(); i++)
	{
		const std::string& id = rtMeshListJson[i].value("model-id", "");
		const std::string& file = rtMeshListJson[i]["file"];

		const std::vector<std::array<float, 16>>& preTransforms = rtMeshListJson[i]["pre-transforms"];
		const std::vector<mtd::Mat4x4>* pPreTransforms =
			reinterpret_cast<const std::vector<mtd::Mat4x4>*>(&preTransforms);

		pMeshManager->createNewMesh(i, id.c_str(), file.c_str(), *pPreTransforms);
	}
}
