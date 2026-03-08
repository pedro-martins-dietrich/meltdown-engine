#include <pch.hpp>
#include "RayTracingMeshManager.hpp"

mtd::RayTracingMeshManager::RayTracingMeshManager(const Device& device, const MaterialInfo& materialInfo)
	: BaseMeshManager{device},
	accelerationStructureWriteOp{},
	vertexBuffer
	{
		device,
		vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress |
			vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR,
		vk::MemoryPropertyFlagBits::eDeviceLocal
	},
	indexBuffer
	{
		device,
		vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress |
			vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR,
		vk::MemoryPropertyFlagBits::eDeviceLocal
	},
	blas{device},
	tlas{device},
	materialIndexBuffer{device, vk::BufferUsageFlagBits::eStorageBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal},
	materialLump{device, materialInfo},
	vertexCount{0},
	triangleCount{0},
	currentIndexOffset{0},
	currentMaterialIndexOffset{0}
{}

void mtd::RayTracingMeshManager::loadMeshes(DescriptorSetHandler& meshDescriptorSetHandler)
{
	for(uint32_t i = 0; i < meshes.size(); i++)
	{
		loadMeshToLump(meshes[i]);
		meshIndexMap[meshes[i].getModelID()] = i;
	}
	loadMeshesToGPU(commandHandler);

	createAccelerationStructure();

	meshDescriptorSetHandler
		.assignExternalResourcesToDescriptor(0, 0, tlas.getBuffer(), &accelerationStructureWriteOp);
	meshDescriptorSetHandler.assignExternalResourcesToDescriptor(0, 3, vertexBuffer);
	meshDescriptorSetHandler.assignExternalResourcesToDescriptor(0, 4, indexBuffer);
	meshDescriptorSetHandler.assignExternalResourcesToDescriptor(0, 5, materialIndexBuffer);
	materialLump.assignFloatDataBufferToDescriptor(meshDescriptorSetHandler, 0, 6);
	if(meshDescriptorSetHandler.getBindingCount() == 8)
		materialLump.assignTexturesToDescriptor(meshDescriptorSetHandler, 0, 7);
}

void mtd::RayTracingMeshManager::createNewMesh
(
	uint32_t index, const char* id, const char* file, const std::vector<Mat4x4>& preTransforms
)
{
	meshes.emplace_back(device, index, id, file, materialLump, preTransforms);
}

void mtd::RayTracingMeshManager::rayTraceMesh
(
	const vk::CommandBuffer& commandBuffer,
	const RayTracingPipeline& rayTracingPipeline,
	const vk::detail::DispatchLoaderDynamic& dldi
) const
{
	rayTracingPipeline.traceRays(commandBuffer, dldi);
}

void mtd::RayTracingMeshManager::createAccelerationStructure()
{
	blas.createBLAS
	(
		commandHandler, vertexBuffer.getBufferAddress(), vertexCount, indexBuffer.getBufferAddress(), triangleCount
	);

	GpuBuffer instanceBuffer = blas.createInstanceBuffer();

	tlas.createTLAS(commandHandler, instanceBuffer.getBufferAddress());

	accelerationStructureWriteOp.accelerationStructureCount = 1U;
	accelerationStructureWriteOp.pAccelerationStructures = &tlas.getAccelerationStructure();
}

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

	vertexCount = vertexLump.size();
	triangleCount = indexLump.size() / 3;

	vertexLump.clear();
	indexLump.clear();
	materialIndexLump.clear();
	currentIndexOffset = 0;
}
