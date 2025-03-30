#include <pch.hpp>
#include "RayTracingMeshManager.hpp"

mtd::RayTracingMeshManager::RayTracingMeshManager(const Device& device)
	: BaseMeshManager{device},
	currentIndexOffset{0},
	currentMaterialIndexOffset{0},
	vertexBuffer{device, vk::BufferUsageFlagBits::eStorageBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal},
	indexBuffer{device, vk::BufferUsageFlagBits::eStorageBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal},
	materialIndexBuffer{device, vk::BufferUsageFlagBits::eStorageBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal}
{}

uint32_t mtd::RayTracingMeshManager::getMaterialCount() const
{
	uint32_t totalMaterialCount = 0;
	for(const RayTracingMesh& mesh: meshes)
		totalMaterialCount += mesh.getMaterialCount();
	return totalMaterialCount;
}

uint32_t mtd::RayTracingMeshManager::getTextureCount() const
{
	uint32_t totalTextureCount = 0;
	for(const RayTracingMesh& mesh: meshes)
		totalTextureCount += mesh.getTextureCount();
	return totalTextureCount;
}

// Checks if the material type for the stored meshes has float data
bool mtd::RayTracingMeshManager::hasMaterialFloatData() const
{
	if(meshes.empty()) return false;
	return meshes[0].hasMaterialFloatData();
}

// Loads the materials and groups the meshes into a lump, then passes the data to the GPU
void mtd::RayTracingMeshManager::loadMeshes(DescriptorSetHandler& meshDescriptorSetHandler)
{
	uint32_t currentMaterialCount = 0;
	for(uint32_t i = 0; i < meshes.size(); i++)
	{
		loadMeshToLump(meshes[i]);

		meshes[i].loadMaterials(device, commandHandler, meshDescriptorSetHandler, currentMaterialCount);
		currentMaterialCount += meshes[i].getMaterialCount();

		meshIndexMap[meshes[i].getModelID()] = i;
	}
	loadMeshesToGPU(commandHandler);

	meshDescriptorSetHandler.assignExternalResourcesToDescriptor(0, 1, vertexBuffer);
	meshDescriptorSetHandler.assignExternalResourcesToDescriptor(0, 2, indexBuffer);
	meshDescriptorSetHandler.assignExternalResourcesToDescriptor(0, 3, materialIndexBuffer);
}

// Binds vertex and index buffers
void mtd::RayTracingMeshManager::bindBuffers(const vk::CommandBuffer& commandBuffer) const
{}

// Draws the meshes using a rasterization pipeline
void mtd::RayTracingMeshManager::drawMesh
(
	const vk::CommandBuffer& commandBuffer, const GraphicsPipeline& graphicsPipeline
) const
{}

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
void mtd::RayTracingMeshManager::loadMeshesToGPU(const CommandHandler& commandHandler)
{
	vertexBuffer.createDeviceLocal(commandHandler, sizeof(Vertex) * vertexLump.size(), vertexLump.data());
	indexBuffer.createDeviceLocal(commandHandler, sizeof(uint32_t) * indexLump.size(), indexLump.data());
	materialIndexBuffer
		.createDeviceLocal(commandHandler, sizeof(uint16_t) * materialIndexLump.size(), materialIndexLump.data());

	for(RayTracingMesh& mesh: meshes)
		mesh.createInstanceBuffer();

	vertexLump.clear();
	indexLump.clear();
	materialIndexLump.clear();
	currentIndexOffset = 0;
}
