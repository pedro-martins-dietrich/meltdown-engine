#include <pch.hpp>
#include "RayTracingMeshManager.hpp"

#include "../../../Utils/Logger.hpp"

mtd::RayTracingMeshManager::RayTracingMeshManager(const Device& device, const MaterialInfo& materialInfo)
	: BaseMeshManager{device},
	currentIndexOffset{0},
	currentMaterialIndexOffset{0},
	accelerationStructure{nullptr},
	accelerationStructureWriteOp{},
	accelerationStructureBuffer
	{
		device,
		1024,
		vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress,
		vk::MemoryPropertyFlagBits::eDeviceLocal
	},
	vertexBuffer{device, vk::BufferUsageFlagBits::eStorageBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal},
	indexBuffer{device, vk::BufferUsageFlagBits::eStorageBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal},
	materialIndexBuffer{device, vk::BufferUsageFlagBits::eStorageBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal},
	materialLump{device, materialInfo}
{
	createAccelerationStructure();
}

mtd::RayTracingMeshManager::~RayTracingMeshManager()
{
	device.getDevice().destroyAccelerationStructureKHR(accelerationStructure, nullptr, device.getDLDI());
}

// Loads the materials and groups the meshes into a lump, then passes the data to the GPU
void mtd::RayTracingMeshManager::loadMeshes(DescriptorSetHandler& meshDescriptorSetHandler)
{
	for(uint32_t i = 0; i < meshes.size(); i++)
	{
		loadMeshToLump(meshes[i]);
		meshIndexMap[meshes[i].getModelID()] = i;
	}
	loadMeshesToGPU(commandHandler);

	meshDescriptorSetHandler
		.assignExternalResourcesToDescriptor(0, 0, accelerationStructureBuffer, &accelerationStructureWriteOp);
	meshDescriptorSetHandler.assignExternalResourcesToDescriptor(0, 2, vertexBuffer);
	meshDescriptorSetHandler.assignExternalResourcesToDescriptor(0, 3, indexBuffer);
	meshDescriptorSetHandler.assignExternalResourcesToDescriptor(0, 4, materialIndexBuffer);
	materialLump.assignFloatDataBufferToDescriptor(meshDescriptorSetHandler, 0, 5);
	if(meshDescriptorSetHandler.getBindingCount() == 7)
		materialLump.assignTexturesToDescriptor(meshDescriptorSetHandler, 0, 6);
}

// Creates a new ray tracing mesh
void mtd::RayTracingMeshManager::createNewMesh
(
	uint32_t index, const char* id, const char* file, const std::vector<Mat4x4>& preTransforms
)
{
	meshes.emplace_back(device, index, id, file, materialLump, preTransforms);
}

// Draws the meshes using a ray tracing pipeline
void mtd::RayTracingMeshManager::rayTraceMesh
(
	const vk::CommandBuffer& commandBuffer,
	const RayTracingPipeline& rayTracingPipeline,
	const vk::detail::DispatchLoaderDynamic& dldi
) const
{
	rayTracingPipeline.traceRays(commandBuffer, dldi);
}

// Creates the acceleration structure
void mtd::RayTracingMeshManager::createAccelerationStructure()
{
	vk::AccelerationStructureCreateInfoKHR asCreateInfo{};
	asCreateInfo.createFlags = vk::AccelerationStructureCreateFlagsKHR();
	asCreateInfo.buffer = accelerationStructureBuffer.getBuffer();
	asCreateInfo.offset = 0;
	asCreateInfo.size = accelerationStructureBuffer.getSize();
	asCreateInfo.type = vk::AccelerationStructureTypeKHR::eTopLevel;
	asCreateInfo.deviceAddress = 0;

	vk::Result result = device.getDevice()
		.createAccelerationStructureKHR(&asCreateInfo, nullptr, &accelerationStructure, device.getDLDI());
	if(result != vk::Result::eSuccess)
		LOG_ERROR("Failed to create acceleration structure. Vulkan result: %d", result);

	accelerationStructureWriteOp.accelerationStructureCount = 1;
	accelerationStructureWriteOp.pAccelerationStructures = &accelerationStructure;
}

// Stores a mesh in the lump of data
void mtd::RayTracingMeshManager::loadMeshToLump(RayTracingMesh& mesh)
{
	const std::vector<Vertex>& vertices = mesh.getVertices();
	const std::vector<uint32_t>& indices = mesh.getIndices();
	const std::vector<uint16_t>& materialIndices = mesh.getMaterialIndices();

	vertexLump.insert(vertexLump.end(), vertices.begin(), vertices.end());

	indexLump.reserve(indexLump.size() + indices.size());
	for(uint32_t index: indices)
		indexLump.push_back(index + currentIndexOffset);
	currentIndexOffset += vertices.size();

	materialIndexLump.reserve(materialIndexLump.size() + materialIndices.size());
	for(uint16_t materialID: materialIndices)
		materialIndexLump.push_back(materialID + currentMaterialIndexOffset);
	currentMaterialIndexOffset += mesh.getMaterialCount();

	mesh.clearMeshData();
}

// Loads the lumps into the VRAM and clears them
void mtd::RayTracingMeshManager::loadMeshesToGPU(const CommandHandler& traceRaysCommandHandler)
{
	vertexBuffer.createDeviceLocal(traceRaysCommandHandler, sizeof(Vertex) * vertexLump.size(), vertexLump.data());
	indexBuffer.createDeviceLocal(traceRaysCommandHandler, sizeof(uint32_t) * indexLump.size(), indexLump.data());
	materialIndexBuffer.createDeviceLocal
	(
		traceRaysCommandHandler, sizeof(uint16_t) * materialIndexLump.size(), materialIndexLump.data()
	);
	materialLump.loadMaterialsToGPU(traceRaysCommandHandler);

	for(RayTracingMesh& mesh: meshes)
		mesh.createInstanceBuffer();

	vertexLump.clear();
	indexLump.clear();
	materialIndexLump.clear();
	currentIndexOffset = 0;
}
