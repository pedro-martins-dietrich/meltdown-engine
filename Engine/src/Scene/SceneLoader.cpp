#include <pch.hpp>
#include "SceneLoader.hpp"

#include <nlohmann/json.hpp>

#include <Meltdown.hpp>

#include "../Vulkan/Mesh/RayTracingMesh/RayTracingMeshManager.hpp"
#include "../Utils/FileHandler.hpp"
#include "../Utils/Logger.hpp"

namespace mtd::SceneLoader
{
	static constexpr const char* SCENE_LOADER_VERSION = "0.2.1";

	// Loads initial camera data for the scene
	static void loadCamera(const nlohmann::json& cameraJson);

	// Loads the GPU resources from the scene file and allocates them
	static void loadGpuResources(const nlohmann::json& resourcesInfoJson, ResourceManager& resourceManager);

	// Loads descriptor info from the scene file
	static void loadDescriptors(const nlohmann::json& descriptorsJson, DescriptorManager& descriptorManager);

	// Fetches the framebuffer info from the scene file
	static void loadFramebuffer(const nlohmann::json& framebufferJson, std::vector<FramebufferInfo>& framebufferInfos);
	// Fetches the rasterization pipeline infos from the scene file
	static void loadRasterizationPipelines
	(
		const nlohmann::json& rasterizationPipelineJson,
		std::vector<RasterizationPipelineInfo>& rasterizationPipelineInfos,
		std::vector<RenderPassInfo>& renderOrder,
		const DescriptorManager& descriptorManager
	);
	// Fetches the ray tracing pipeline infos from the scene file
	static void loadRayTracingPipelines
	(
		const nlohmann::json& rayTracingPipelineJson, std::vector<RayTracingPipelineInfo>& rayTracingPipelineInfos
	);
	// Fetches the compute pipeline infos from the scene file
	static void loadComputePipelines
	(
		const nlohmann::json& computePipelineJson, std::vector<ComputePipelineInfo>& computePipelineInfos
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
		const nlohmann::json& descriptorSetInfoJson, std::vector<DescriptorInfo>& descriptorInfos
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
	static void loadTextures
	(
		const nlohmann::json& texturesJson, ResourceManager& resourceManager, TexturePool& texturePool
	);
	// Fetches all material files and material sets from the scene file and loads them
	static void loadMaterials
	(
		const nlohmann::json& materialsJson,
		const nlohmann::json& materialSetsJson,
		ResourceManager& resourceManager,
		MaterialManager& materialManager
	);
	// Fetches all mesh files from the scene file and loads them
	static void loadMeshes(const nlohmann::json& meshJson, ResourceManager& resourceManager, MeshPool& meshPool);

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
	ResourceManager& resourceManager,
	DescriptorManager& descriptorManager,
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
	loadGpuResources(sceneJson["gpu-resources"], resourceManager);
	loadDescriptors(sceneJson["descriptor-sets"], descriptorManager);

	const nlohmann::json& framebuffersJson = sceneJson["framebuffers"];
	const nlohmann::json& rasterPipelinesJson = sceneJson["rasterization-pipelines"];
	const nlohmann::json& computePipelinesJson = sceneJson["compute-pipelines"];
	const nlohmann::json& rtPipelinesJson = sceneJson["ray-tracing-pipelines"];
	const nlohmann::json& fbPipelinesJson = sceneJson["framebuffer-pipelines"];
	const nlohmann::json& meshesJson = sceneJson["meshes-old"];

	if(rtPipelinesJson.size() != meshesJson.size())
		LOG_ERROR("The number of ray tracing pipelines and mesh managers should be the same.");
	
	framebufferInfos.reserve(framebuffersJson.size());
	pipelineInfos.rasterizerInfos.reserve(rasterPipelinesJson.size());
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
		loadRasterizationPipelines(rasterPipelineJson, pipelineInfos.rasterizerInfos, renderOrder, descriptorManager);
	for(const nlohmann::json& computePipelineJson: computePipelinesJson)
		loadComputePipelines(computePipelineJson, pipelineInfos.computeInfos);
	for(const nlohmann::json& rtPipelineJson: rtPipelinesJson)
		loadRayTracingPipelines(rtPipelineJson, pipelineInfos.rayTracingInfos);
	for(const nlohmann::json& fbPipelineJson: fbPipelinesJson)
		loadFramebufferPipelines(fbPipelineJson, pipelineInfos.framebufferInfos, renderOrder);

	for(uint32_t i = 0U; i < meshesJson.size(); i++)
	{
		if(i >= pipelineInfos.rasterizerInfos.size())
		{
			uint32_t rtPipelineIndex = i - rasterPipelinesJson.size();
			loadRayTracingMeshes(device, meshesJson[i], pipelineInfos.rayTracingInfos[rtPipelineIndex], meshManagers);
			continue;
		}
	}

	loadTextures(sceneJson["textures"], resourceManager, texturePool);
	loadMaterials(sceneJson["materials"], sceneJson["material-sets"], resourceManager, materialManager);
	loadMeshes(sceneJson["meshes"], resourceManager, meshPool);

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

void mtd::SceneLoader::loadGpuResources(const nlohmann::json& resourcesInfoJson, ResourceManager& resourceManager)
{
	const nlohmann::json& buffersJson = resourcesInfoJson["buffers"];
	const nlohmann::json& imagesJson = resourcesInfoJson["images"];

	for(const nlohmann::json& bufferJson: buffersJson)
	{
		resourceManager.createBuffer
		(
			bufferJson.value("name", ""),
			static_cast<GpuBufferType>(bufferJson.value("type", 0U)),
			static_cast<GpuMemoryUsage>(bufferJson.value("memory-usage", 0U)),
			bufferJson.value("size", 0UL)
		);
	}
	for(const nlohmann::json& imageJson: imagesJson)
	{
		resourceManager.createImage
		(
			imageJson.value("name", ""),
			UIntVec2{imageJson.value("size-x", 0U), imageJson.value("size-y", 0U)},
			static_cast<vk::Format>(imageJson.value("format", 37U)),
			static_cast<vk::ImageUsageFlags>(imageJson.value("type", 4U)),
			Vec2{imageJson.value("resolution-ratio-x", -1.0f), imageJson.value("resolution-ratio-y", -1.0f)}
		);
	}

	LOG_VERBOSE
	(
		"Loaded %d GPU resources: %d buffers - %d images.",
		(buffersJson.size() + imagesJson.size()),
		buffersJson.size(), imagesJson.size()
	);
}

void mtd::SceneLoader::loadDescriptors(const nlohmann::json& descriptorsJson, DescriptorManager& descriptorManager)
{
	std::vector<DescriptorInfo> descriptors;
	for(const nlohmann::json& setJson: descriptorsJson)
	{
		for(const nlohmann::json& descJson: setJson.at("descriptors"))
		{
			descriptors.emplace_back
			(
				descJson.value("resource-name", ""),
				descJson.value<DescriptorType>("type", DescriptorType::UniformBuffer),
				static_cast<ShaderStage>(descJson.value("shader-stage", 63U)),
				descJson.value("count", 1U)
			);
		}
		descriptorManager.createSet(descriptors);
		descriptors.clear();
	}
	descriptorManager.allocate();

	LOG_VERBOSE("Loaded %d descriptor sets.", descriptorsJson.size());
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
	const nlohmann::json& rasterizationPipelineJson,
	std::vector<RasterizationPipelineInfo>& rasterizationPipelineInfos,
	std::vector<RenderPassInfo>& renderOrder,
	const DescriptorManager& descriptorManager
)
{
	std::vector<DescriptorInfo> descriptorInfos;
	loadDescriptorInfos(rasterizationPipelineJson["descriptor-set-info"], descriptorInfos);

	int32_t targetFramebuffer = rasterizationPipelineJson["target-framebuffer"];
	if(targetFramebuffer == -1)
		renderOrder.back().pipelineIndices.emplace_back(static_cast<uint32_t>(rasterizationPipelineInfos.size()));
	else
		renderOrder[targetFramebuffer].pipelineIndices
			.emplace_back(static_cast<uint32_t>(rasterizationPipelineInfos.size()));

	rasterizationPipelineInfos.emplace_back(RasterizationPipelineInfo
	{
		{
			rasterizationPipelineJson["name"],
			rasterizationPipelineJson.value<std::vector<DescriptorSetID>>("descriptor-sets", {}),
			std::move(descriptorInfos)
		},
		rasterizationPipelineJson["vertex-shader"],
		rasterizationPipelineJson["fragment-shader"],
		static_cast<MeshType>(rasterizationPipelineJson["mesh-type"]),
		targetFramebuffer,
		static_cast<ShaderPrimitiveTopology>(rasterizationPipelineJson["shader-primitive-topology"]),
		static_cast<ShaderFaceCulling>(rasterizationPipelineJson["shader-face-culling"]),
		rasterizationPipelineJson["transparency"],
	});
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

	rayTracingPipelineInfos.emplace_back(RayTracingPipelineInfo
	{
		{
			rtPipelineJson["name"],
			rtPipelineJson.value<std::vector<DescriptorSetID>>("descriptor-sets", {}),
			std::move(descriptorInfos)
		},
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
	});
}

void mtd::SceneLoader::loadComputePipelines
(
	const nlohmann::json& computePipelineJson, std::vector<ComputePipelineInfo>& computePipelineInfos
)
{
	std::vector<DescriptorInfo> descriptorInfos;
	loadDescriptorInfos(computePipelineJson["descriptor-set-info"], descriptorInfos);

	computePipelineInfos.emplace_back(ComputePipelineInfo
	{
		{
			computePipelineJson["name"],
			computePipelineJson.value<std::vector<DescriptorSetID>>("descriptor-sets", {}),
			std::move(descriptorInfos)
		},
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
	});
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

	framebufferPipelineInfos.emplace_back(FramebufferPipelineInfo
	{
		{
			fbPipelineJson["name"],
			fbPipelineJson.value<std::vector<DescriptorSetID>>("descriptor-sets", {}),
			std::move(descriptorInfos)
		},
		fbPipelineJson["vertex-shader"],
		fbPipelineJson["fragment-shader"],
		targetFramebuffer,
		std::move(inputAttachments),
		fbPipelineJson["compute-storage-images"],
		fbPipelineJson["ray-tracing-storage-images"],
		fbPipelineJson["dependencies"]
	});
}

void mtd::SceneLoader::loadDescriptorInfos
(
	const nlohmann::json& descriptorSetInfoJson, std::vector<DescriptorInfo>& descriptorInfos
)
{
	descriptorInfos.reserve(descriptorSetInfoJson.size());
	for(const nlohmann::json& descriptorInfoJson: descriptorSetInfoJson)
	{
		descriptorInfos.emplace_back(DescriptorInfo
		{
			"",
			static_cast<DescriptorType>(descriptorInfoJson.value("descriptor-type", 0U)),
			static_cast<ShaderStage>(descriptorInfoJson.value("shader-stage", 2U)),
			descriptorInfoJson.value("descriptor-count", 1U)
		});
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

void mtd::SceneLoader::loadTextures
(
	const nlohmann::json& texturesJson, ResourceManager& resourceManager, TexturePool& texturePool
)
{
	std::vector<std::string> textureInfos;
	textureInfos.reserve(texturesJson.size());
	for(const std::string& path: texturesJson)
		textureInfos.emplace_back(MTD_RESOURCES_PATH + path);

	texturePool.loadTextures(resourceManager, textureInfos);

	LOG_VERBOSE("Loaded %d textures.", textureInfos.size());
}

void mtd::SceneLoader::loadMaterials
(
	const nlohmann::json& materialsJson,
	const nlohmann::json& materialSetsJson,
	ResourceManager& resourceManager,
	MaterialManager& materialManager
)
{
	std::vector<std::string> materialPaths;
	materialPaths.reserve(materialsJson.size());
	for(const std::string& path: materialsJson)
		materialPaths.emplace_back(MTD_RESOURCES_PATH + path);

	std::vector<std::vector<uint32_t>> materialSets;
	materialSetsJson.get_to(materialSets);

	materialManager.loadMaterials(resourceManager, materialPaths, materialSets);
}

void mtd::SceneLoader::loadMeshes(const nlohmann::json& meshJson, ResourceManager& resourceManager, MeshPool& meshPool)
{
	std::vector<std::string> meshPaths;
	meshPaths.reserve(meshJson.size());
	for(const std::string& path: meshJson)
		meshPaths.emplace_back(MTD_RESOURCES_PATH + path);

	meshPool.loadMeshes(resourceManager, meshPaths);
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
