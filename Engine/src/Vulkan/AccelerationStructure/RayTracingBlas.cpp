#include <pch.hpp>
#include "RayTracingBlas.hpp"

#include "../../Utils/Logger.hpp"

mtd::RayTracingBlas::RayTracingBlas
(
	const Device& mtdDevice, ResourceManager& resourceManager,
	const CommandHandler& commandHandler, const MeshData& meshData
) : mtdDevice{mtdDevice}, resourceManager{resourceManager}, accelerationStructure{nullptr}
{
	vk::DeviceAddress vertexAddress = resourceManager.getBufferAddress(resourceManager.getResourceID("VertexBuffer"));
	vk::DeviceAddress indexAddress = resourceManager.getBufferAddress(resourceManager.getResourceID("IndexBuffer"));

	size_t geometryCount = meshData.submeshes.size();
	std::vector<uint32_t> primitiveCounts(geometryCount);
	std::vector<uint32_t> primitiveOffsets(geometryCount);
	std::vector<vk::AccelerationStructureGeometryKHR> geometries(geometryCount);

	for(size_t i = 0; i < geometryCount; i++)
	{
		vk::AccelerationStructureGeometryTrianglesDataKHR asGeometryTrianglesData{};
		asGeometryTrianglesData.vertexFormat = vk::Format::eR32G32B32Sfloat;
		asGeometryTrianglesData.vertexData.deviceAddress = vertexAddress + meshData.vertexOffset;
		asGeometryTrianglesData.vertexStride = meshData.vertexStride;
		asGeometryTrianglesData.maxVertex = meshData.vertexCount - 1U;
		asGeometryTrianglesData.indexType = vk::IndexType::eUint32;
		asGeometryTrianglesData.indexData.deviceAddress = indexAddress;
		asGeometryTrianglesData.transformData.deviceAddress = 0UL;
		asGeometryTrianglesData.pNext = nullptr;

		geometries[i].geometryType = vk::GeometryTypeKHR::eTriangles;
		geometries[i].geometry.triangles = asGeometryTrianglesData;
		geometries[i].flags = vk::GeometryFlagBitsKHR::eOpaque;
		geometries[i].pNext = nullptr;

		primitiveCounts[i] = meshData.submeshes[i].indexCount / 3U;
		primitiveOffsets[i] = meshData.submeshes[i].indexOffset * sizeof(uint32_t);
	}

	createAndBuild(commandHandler, geometries, primitiveCounts, primitiveOffsets);
}

mtd::RayTracingBlas::~RayTracingBlas()
{
	if(accelerationStructure)
		mtdDevice.getDevice().destroyAccelerationStructureKHR(accelerationStructure, nullptr, mtdDevice.getDLDI());
	resourceManager.deleteResource(bufferID);
}

mtd::RayTracingBlas::RayTracingBlas(RayTracingBlas&& other) noexcept
	: mtdDevice{other.mtdDevice}, resourceManager{other.resourceManager},
	accelerationStructure{other.accelerationStructure}, bufferID{other.bufferID}
{
	other.accelerationStructure = nullptr;
	other.bufferID = 0U;
}

void mtd::RayTracingBlas::createAndBuild
(
	const CommandHandler& commandHandler,
	const std::vector<vk::AccelerationStructureGeometryKHR>& geometries,
	const std::vector<uint32_t>& primitiveCounts,
	const std::vector<uint32_t>& primitiveOffsets
)
{
	assert
	(
		geometries.size() == primitiveCounts.size()
		&& geometries.size() == primitiveOffsets.size()
		&& "The count for geometries, primitive counts and offsets must be the same."
	);

	vk::AccelerationStructureBuildGeometryInfoKHR buildGeometryInfo;
	buildGeometryInfo.type = vk::AccelerationStructureTypeKHR::eBottomLevel;
	buildGeometryInfo.flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace;
	buildGeometryInfo.mode = vk::BuildAccelerationStructureModeKHR::eBuild;
	buildGeometryInfo.srcAccelerationStructure = nullptr;
	buildGeometryInfo.dstAccelerationStructure = nullptr;
	buildGeometryInfo.geometryCount = geometries.size();
	buildGeometryInfo.pGeometries = geometries.data();
	buildGeometryInfo.ppGeometries = nullptr;

	vk::AccelerationStructureBuildSizesInfoKHR buildSizesInfo{};
	mtdDevice.getDevice().getAccelerationStructureBuildSizesKHR
	(
		vk::AccelerationStructureBuildTypeKHR::eDevice, &buildGeometryInfo,
		primitiveCounts.data(), &buildSizesInfo, mtdDevice.getDLDI()
	);
	bufferID = resourceManager.createBuffer
	(
		"", GpuBufferType::AccelerationStructure, GpuMemoryUsage::GpuOnly,
		buildSizesInfo.accelerationStructureSize
	);

	vk::AccelerationStructureCreateInfoKHR asCreateInfo{};
	asCreateInfo.createFlags = vk::AccelerationStructureCreateFlagsKHR();
	asCreateInfo.buffer = resourceManager.getVulkanBuffer(bufferID);
	asCreateInfo.offset = 0UL;
	asCreateInfo.size = buildSizesInfo.accelerationStructureSize;
	asCreateInfo.type = vk::AccelerationStructureTypeKHR::eBottomLevel;
	asCreateInfo.deviceAddress = 0UL;

	vk::Result result = mtdDevice.getDevice()
		.createAccelerationStructureKHR(&asCreateInfo, nullptr, &accelerationStructure, mtdDevice.getDLDI());
	if(result != vk::Result::eSuccess)
		LOG_ERROR("Failed to create bottom level acceleration structure. Vulkan result: %d", result);

	ResourceID scratchBufferID = resourceManager.createBuffer
	(
		"", GpuBufferType::Storage | GpuBufferType::DeviceAddress,
		GpuMemoryUsage::GpuOnly, buildSizesInfo.buildScratchSize
	);

	buildGeometryInfo.dstAccelerationStructure = accelerationStructure;
	buildGeometryInfo.scratchData.deviceAddress = resourceManager.getBufferAddress(scratchBufferID);

	std::vector<vk::AccelerationStructureBuildRangeInfoKHR> buildRangeInfos(primitiveCounts.size());
	for(size_t i = 0; i < primitiveCounts.size(); i++)
	{
		buildRangeInfos[i].primitiveCount = primitiveCounts[i];
		buildRangeInfos[i].primitiveOffset = primitiveOffsets[i];
		buildRangeInfos[i].firstVertex = 0U;
		buildRangeInfos[i].transformOffset = 0U;
	}
	const vk::AccelerationStructureBuildRangeInfoKHR* pBuildRangeInfo = buildRangeInfos.data();

	vk::CommandBuffer commandBuffer = commandHandler.beginSingleTimeCommand();
	commandBuffer.buildAccelerationStructuresKHR(1U, &buildGeometryInfo, &pBuildRangeInfo, mtdDevice.getDLDI());
	commandHandler.endSingleTimeCommand(commandBuffer);

	resourceManager.deleteResource(scratchBufferID);
}
