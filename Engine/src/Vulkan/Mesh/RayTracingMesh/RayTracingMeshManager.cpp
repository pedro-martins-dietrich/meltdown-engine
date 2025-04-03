#include <pch.hpp>
#include "RayTracingMeshManager.hpp"

mtd::RayTracingMeshManager::RayTracingMeshManager(const Device& device, const MaterialInfo& materialInfo)
	: BaseMeshManager{device},
	currentIndexOffset{0},
	currentMaterialIndexOffset{0},
	vertexBuffer{device, vk::BufferUsageFlagBits::eStorageBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal},
	indexBuffer{device, vk::BufferUsageFlagBits::eStorageBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal},
	materialIndexBuffer{device, vk::BufferUsageFlagBits::eStorageBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal},
	materialLump{device, materialInfo}
{}

// Loads the materials and groups the meshes into a lump, then passes the data to the GPU
void mtd::RayTracingMeshManager::loadMeshes(DescriptorSetHandler& meshDescriptorSetHandler)
{
	for(uint32_t i = 0; i < meshes.size(); i++)
	{
		loadMeshToLump(meshes[i]);
		meshIndexMap[meshes[i].getModelID()] = i;
	}
	loadMeshesToGPU(commandHandler);

	meshDescriptorSetHandler.assignExternalResourcesToDescriptor(0, 1, vertexBuffer);
	meshDescriptorSetHandler.assignExternalResourcesToDescriptor(0, 2, indexBuffer);
	meshDescriptorSetHandler.assignExternalResourcesToDescriptor(0, 3, materialIndexBuffer);
	materialLump.assignBufferToDescriptor(meshDescriptorSetHandler, 0, 4);
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
