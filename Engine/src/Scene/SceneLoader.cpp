#include <pch.hpp>
#include "SceneLoader.hpp"

#include <nlohmann/json.hpp>

#include <Meltdown.hpp>

#include "../Vulkan/Mesh/RayTracingMesh/RayTracingMeshManager.hpp"
#include "../Utils/FileHandler.hpp"
#include "../Utils/Logger.hpp"

namespace mtd::SceneLoader
{
	static constexpr const char* SCENE_LOADER_VERSION = "0.2.0";

	// Loads initial camera data for the scene
	static void loadCamera(const nlohmann::json& cameraJson);
	// Fetches the framebuffer info from the scene file
	static void loadFramebuffer
	(
		const nlohmann::json& framebufferJson,
		std::vector<FramebufferInfo>& framebufferInfos
	);
	// Fetches the rasterization pipeline infos from the scene file
	static void loadRasterizationPipelines
	(
		const nlohmann::json& pipelineJson,
		std::vector<GraphicsPipelineInfo>& graphicsPipelineInfos,
		std::vector<RenderPassInfo>& renderOrder
	);
	// Fetches the ray tracing pipeline infos from the scene file
	static void loadRayTracingPipelines
	(
		const nlohmann::json& rayTracingPipelineJson,
		std::vector<RayTracingPipelineInfo>& rayTracingPipelineInfos
	);
	// Fetches the compute pipeline infos from the scene file
	static void loadComputePipelines
	(
		const nlohmann::json& computePipelineJson,
		std::vector<ComputePipelineInfo>& computePipelineInfos
	);
	// Fetches the framebuffer pipeline info from the scene file
	static void loadFramebufferPipelines
	(
		const nlohmann::json& framebufferPipelineJson,
		std::vector<FramebufferPipelineInfo>& framebufferPipelineInfos,
		std::vector<RenderPassInfo>& renderOrder
	);

	// Loads the descriptor set info for a pipeline
	static void loadDescriptorInfos
	(
		const nlohmann::json& descriptorSetInfoJson,
		std::vector<DescriptorInfo>& descriptorInfos
	);

	// Fetches ray tracing meshes from the scene file
	static void loadRayTracingMeshes
	(
		const Device& device,
		const nlohmann::json& rtMeshListJson,
		const RayTracingPipelineInfo& rtPipelineInfo,
		std::vector<std::unique_ptr<MeshManager>>& meshManagers
	);

	// Fetches all textures from the scene file
	static void loadTextures(const nlohmann::json& texturesJson, TexturePool& texturePool);
	// Fetches all material files and material sets from the scene file and loads them
	static void loadMaterials
	(
		const nlohmann::json& materialsJson, const nlohmann::json& materialSetsJson, MaterialManager& materialManager
	);
	// Fetches all mesh files from the scene file and loads them
	static void loadMeshes(const nlohmann::json& meshJson, MeshPool& meshPool);

	// Loads all instances from the scene file
	static void loadInstances(const nlohmann::json& instancesJson, InstanceManager& instanceManager);
}

void mtd::SceneLoader::load
(
	const Device& device,
	std::string_view fileName,
	std::vector<FramebufferInfo>& framebufferInfos,
	PipelineInfoBundle& pipelineInfos,
	std::vector<RenderPassInfo>& renderOrder,
	std::vector<std::unique_ptr<MeshManager>>& meshManagers,
	InstanceManager& instanceManager,
	TexturePool& texturePool,
	MaterialManager& materialManager,
	MeshPool& meshPool
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
		LOG_ERROR("Scene JSON file (\"%s\") version is incompatible with the scene loader.", fileName.data());
		return;
	}

	loadCamera(sceneJson["camera"]);

	const nlohmann::json& framebuffersJson = sceneJson["framebuffers"];
	const nlohmann::json& rasterPipelinesJson = sceneJson["rasterization-pipelines"];
	const nlohmann::json& computePipelinesJson = sceneJson["compute-pipelines"];
	const nlohmann::json& rtPipelinesJson = sceneJson["ray-tracing-pipelines"];
	const nlohmann::json& fbPipelinesJson = sceneJson["framebuffer-pipelines"];
	const nlohmann::json& meshesJson = sceneJson["meshes-old"];

	if(rtPipelinesJson.size() != meshesJson.size())
		LOG_ERROR("The number of ray tracing pipelines and mesh managers should be the same.");
	
	framebufferInfos.reserve(framebuffersJson.size());
	pipelineInfos.graphicsInfos.reserve(rasterPipelinesJson.size());
	pipelineInfos.computeInfos.reserve(computePipelinesJson.size());
	pipelineInfos.rayTracingInfos.reserve(rtPipelinesJson.size());
	pipelineInfos.framebufferInfos.reserve(fbPipelinesJson.size());
	renderOrder.reserve(framebuffersJson.size() + 1);
	meshManagers.reserve(meshesJson.size());

	for(int32_t i = 0; i < framebuffersJson.size(); i++)
	{
		loadFramebuffer(framebuffersJson[i], framebufferInfos);
		renderOrder.emplace_back(i);
	}
	renderOrder.emplace_back(-1);

	for(const nlohmann::json& rasterPipelineJson: rasterPipelinesJson)
		loadRasterizationPipelines(rasterPipelineJson, pipelineInfos.graphicsInfos, renderOrder);
	for(const nlohmann::json& computePipelineJson: computePipelinesJson)
		loadComputePipelines(computePipelineJson, pipelineInfos.computeInfos);
	for(const nlohmann::json& rtPipelineJson: rtPipelinesJson)
		loadRayTracingPipelines(rtPipelineJson, pipelineInfos.rayTracingInfos);
	for(const nlohmann::json& fbPipelineJson: fbPipelinesJson)
		loadFramebufferPipelines(fbPipelineJson, pipelineInfos.framebufferInfos, renderOrder);

	for(uint32_t i = 0; i < meshesJson.size(); i++)
	{
		if(i >= pipelineInfos.graphicsInfos.size())
		{
			uint32_t rtPipelineIndex = i - rasterPipelinesJson.size();
			loadRayTracingMeshes(device, meshesJson[i], pipelineInfos.rayTracingInfos[rtPipelineIndex], meshManagers);
			continue;
		}
	}

	loadTextures(sceneJson["textures"], texturePool);
	loadMaterials(sceneJson["materials"], sceneJson["material-sets"], materialManager);
	loadMeshes(sceneJson["meshes"], meshPool);

	loadInstances(sceneJson["instances"], instanceManager);

	LOG_INFO("Scene \"%s\" loaded.", fileName.data());
}

void mtd::SceneLoader::loadCamera(const nlohmann::json& cameraJson)
{
	if(cameraJson["orthographic"])
	{
		EventManager::dispatch<SetOrthographicCameraEvent>(cameraJson["view-width"], cameraJson["far-plane"]);
	}
	else
	{
		EventManager::dispatch<SetPerspectiveCameraEvent>
		(
			cameraJson["fov"], cameraJson["near-plane"], cameraJson["far-plane"]
		);
	}

	CameraHandler::setPosition(Vec3{cameraJson["position"][0], cameraJson["position"][1], cameraJson["position"][2]});
	CameraHandler::setOrientation(cameraJson["yaw"], cameraJson["pitch"]);
}

void mtd::SceneLoader::loadFramebuffer(const nlohmann::json& fbJson, std::vector<FramebufferInfo>& framebufferInfos)
{
	framebufferInfos.emplace_back
	(
		FramebufferInfo
		{
			static_cast<FramebufferAttachments>(fbJson["attachments"]),
			static_cast<TextureSamplingFilterType>(fbJson["sampling-filter"]),
			{fbJson.value("resolution-ratio-horizontal", -1.0f), fbJson.value("resolution-ratio-vertical", -1.0f)},
			fbJson.value("fixed-horizontal-resolution", 1280U), fbJson.value("fixed-vertical-resolution", 720U)
		}
	);
}

void mtd::SceneLoader::loadRasterizationPipelines
(
	const nlohmann::json& graphicsPipelineJson,
	std::vector<GraphicsPipelineInfo>& graphicsPipelineInfos,
	std::vector<RenderPassInfo>& renderOrder
)
{
	std::vector<DescriptorInfo> descriptorInfos;
	loadDescriptorInfos(graphicsPipelineJson["descriptor-set-info"], descriptorInfos);

	std::vector<MaterialFloatDataType> floatDataTypes;
	floatDataTypes.reserve(graphicsPipelineJson["material-float-data-types"].size());
	for(const nlohmann::json& floatDataType: graphicsPipelineJson["material-float-data-types"])
		floatDataTypes.emplace_back(static_cast<MaterialFloatDataType>(floatDataType));

	std::vector<MaterialTextureType> textureTypes;
	textureTypes.reserve(graphicsPipelineJson["material-texture-types"].size());
	for(const nlohmann::json& textureType: graphicsPipelineJson["material-texture-types"])
		textureTypes.emplace_back(static_cast<MaterialTextureType>(textureType));

	int32_t targetFramebuffer = graphicsPipelineJson["target-framebuffer"];
	if(targetFramebuffer == -1)
		renderOrder.back().pipelineIndices.emplace_back(static_cast<uint32_t>(graphicsPipelineInfos.size()));
	else
		renderOrder[targetFramebuffer].pipelineIndices.emplace_back(static_cast<uint32_t>(graphicsPipelineInfos.size()));

	graphicsPipelineInfos.emplace_back
	(
		GraphicsPipelineInfo
		{
			{graphicsPipelineJson["name"], std::move(descriptorInfos)},
			graphicsPipelineJson["vertex-shader"],
			graphicsPipelineJson["fragment-shader"],
			static_cast<MeshType>(graphicsPipelineJson["mesh-type"]),
			targetFramebuffer,
			static_cast<ShaderPrimitiveTopology>(graphicsPipelineJson["shader-primitive-topology"]),
			static_cast<ShaderFaceCulling>(graphicsPipelineJson["shader-face-culling"]),
			graphicsPipelineJson["transparency"],
			std::move(floatDataTypes),
			std::move(textureTypes)
		}
	);
}

void mtd::SceneLoader::loadRayTracingPipelines
(
	const nlohmann::json& rtPipelineJson, std::vector<RayTracingPipelineInfo>& rayTracingPipelineInfos
)
{
	std::vector<DescriptorInfo> descriptorInfos;
	loadDescriptorInfos(rtPipelineJson["descriptor-set-info"], descriptorInfos);

	std::vector<MaterialFloatDataType> floatDataTypes;
	floatDataTypes.reserve(rtPipelineJson["material-float-data-types"].size());
	for(const nlohmann::json& floatDataType: rtPipelineJson["material-float-data-types"])
		floatDataTypes.emplace_back(static_cast<MaterialFloatDataType>(floatDataType));

	std::vector<MaterialTextureType> textureTypes;
	textureTypes.reserve(rtPipelineJson["material-texture-types"].size());
	for(const nlohmann::json& textureType: rtPipelineJson["material-texture-types"])
		textureTypes.emplace_back(static_cast<MaterialTextureType>(textureType));

	rayTracingPipelineInfos.emplace_back
	(
		RayTracingPipelineInfo
		{
			{rtPipelineJson["name"], std::move(descriptorInfos)},
			rtPipelineJson["ray-gen-shader"],
			rtPipelineJson["closest-hit-shader"],
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

void mtd::SceneLoader::loadComputePipelines
(
	const nlohmann::json& computePipelineJson,
	std::vector<ComputePipelineInfo>& computePipelineInfos
)
{
	std::vector<DescriptorInfo> descriptorInfos;
	loadDescriptorInfos(computePipelineJson["descriptor-set-info"], descriptorInfos);

	computePipelineInfos.emplace_back
	(
		ComputePipelineInfo
		{
			{computePipelineJson["name"], std::move(descriptorInfos)},
			computePipelineJson["compute-shader"],
		{
			computePipelineJson.value("resolution-ratio-horizontal", -1.0f),
			computePipelineJson.value("resolution-ratio-vertical", -1.0f)
		},
			{
				computePipelineJson.value("fixed-horizontal-resolution", 1280U),
				computePipelineJson.value("fixed-vertical-resolution", 720U),
			},
			{
				computePipelineJson["workgroups"][0],
				computePipelineJson["workgroups"][1],
				computePipelineJson["workgroups"][2]
			},
			{
				computePipelineJson["workgroup-size"][0],
				computePipelineJson["workgroup-size"][1],
				computePipelineJson["workgroup-size"][2]
			},
			{(computePipelineJson["workgroups"][0] == -1), (computePipelineJson["workgroups"][1] == -1)}
		}
	);
}

void mtd::SceneLoader::loadFramebufferPipelines
(
	const nlohmann::json& fbPipelineJson,
	std::vector<FramebufferPipelineInfo>& framebufferPipelineInfos,
	std::vector<RenderPassInfo>& renderOrder
)
{
	std::vector<DescriptorInfo> descriptorInfos;
	loadDescriptorInfos(fbPipelineJson["descriptor-set-info"], descriptorInfos);

	const nlohmann::json& inputAttachmentsJson = fbPipelineJson["input-attachments"];
	std::vector<AttachmentIdentifier> inputAttachments(inputAttachmentsJson.size());
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
		FramebufferPipelineInfo
		{
			{fbPipelineJson["name"], std::move(descriptorInfos)},
			fbPipelineJson["vertex-shader"],
			fbPipelineJson["fragment-shader"],
			targetFramebuffer,
			std::move(inputAttachments),
			fbPipelineJson["compute-storage-images"],
			fbPipelineJson["ray-tracing-storage-images"],
			fbPipelineJson["dependencies"]
		}
	);
}

void mtd::SceneLoader::loadDescriptorInfos
(
	const nlohmann::json& descriptorSetInfoJson,
	std::vector<DescriptorInfo>& descriptorInfos
)
{
	descriptorInfos.reserve(descriptorSetInfoJson.size());
	for(const nlohmann::json& descriptorInfoJson: descriptorSetInfoJson)
	{
		descriptorInfos.emplace_back
		(
			DescriptorInfo
			{
				static_cast<DescriptorType>(descriptorInfoJson["descriptor-type"]),
				static_cast<ShaderStage>(descriptorInfoJson["shader-stage"]),
				descriptorInfoJson["total-descriptor-size"],
				descriptorInfoJson["descriptor-count"]
			}
		);
	}
}

void mtd::SceneLoader::loadRayTracingMeshes
(
	const Device& device,
	const nlohmann::json& rtMeshListJson,
	const RayTracingPipelineInfo& rtPipelineInfo,
	std::vector<std::unique_ptr<MeshManager>>& meshManagers
)
{
	MaterialInfo materialInfo{rtPipelineInfo.materialFloatDataTypes, rtPipelineInfo.materialTextureTypes};
	meshManagers.emplace_back(std::make_unique<RayTracingMeshManager>(device, materialInfo));

	RayTracingMeshManager* pMeshManager = dynamic_cast<RayTracingMeshManager*>(meshManagers.back().get());

	for(uint32_t i = 0; i < rtMeshListJson.size(); i++)
	{
		const std::string& id = rtMeshListJson[i].value("model-id", "");
		const std::string& file = rtMeshListJson[i]["file"];

		const std::vector<std::array<float, 16>>& preTransforms = rtMeshListJson[i]["pre-transforms"];
		const std::vector<Mat4x4>* pPreTransforms = reinterpret_cast<const std::vector<Mat4x4>*>(&preTransforms);

		pMeshManager->createNewMesh(i, id.c_str(), file.c_str(), *pPreTransforms);
	}
}

void mtd::SceneLoader::loadTextures(const nlohmann::json& texturesJson, TexturePool& texturePool)
{
	std::vector<TextureInfo> textureInfos;
	textureInfos.reserve(texturesJson.size());
	for(const std::string& path: texturesJson)
		textureInfos.emplace_back(MTD_RESOURCES_PATH + path);

	texturePool.loadTextures(textureInfos);

	LOG_VERBOSE("Loaded %d textures.", textureInfos.size());
}

void mtd::SceneLoader::loadMaterials
(
	const nlohmann::json& materialsJson, const nlohmann::json& materialSetsJson, MaterialManager& materialManager
)
{
	std::vector<std::string> materialPaths;
	materialPaths.reserve(materialsJson.size());
	for(const std::string& path: materialsJson)
		materialPaths.emplace_back(MTD_RESOURCES_PATH + path);

	std::vector<std::vector<uint32_t>> materialSets;
	materialSetsJson.get_to(materialSets);

	materialManager.loadMaterials(materialPaths, materialSets);
}

void mtd::SceneLoader::loadMeshes(const nlohmann::json& meshJson, MeshPool& meshPool)
{
	std::vector<std::string> meshPaths;
	meshPaths.reserve(meshJson.size());
	for(const std::string& path: meshJson)
		meshPaths.emplace_back(MTD_RESOURCES_PATH + path);

	meshPool.loadMeshes(meshPaths);
}

void mtd::SceneLoader::loadInstances(const nlohmann::json& instancesJson, InstanceManager& instanceManager)
{
	std::vector<SceneInstance> instances;
	instances.reserve(instancesJson.size());
	for(const nlohmann::json& instance: instancesJson)
	{
		uint32_t pipelineID = instance["pipeline-id"];
		uint32_t meshID = instance["mesh-id"];
		uint32_t materialSetID = instance["material-set-id"];
		const std::array<float, 16>& transform = instance["transform"];
		const Mat4x4* pTransform = reinterpret_cast<const Mat4x4*>(&transform);
		bool visible = instance["visible"];

		instances.emplace_back(*pTransform, pipelineID, meshID, materialSetID, visible);
	}
	instanceManager.loadInstances(instances);
}
