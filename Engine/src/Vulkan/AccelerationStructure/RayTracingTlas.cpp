#include <pch.hpp>
#include "RayTracingTlas.hpp"

#include "../../Utils/Logger.hpp"

mtd::RayTracingTlas::RayTracingTlas(const Device& mtdDevice, ResourceManager& resourceManager)
	: mtdDevice{mtdDevice}, resourceManager{resourceManager}, accelerationStructure{nullptr}
{}

mtd::RayTracingTlas::~RayTracingTlas()
{
	if(accelerationStructure)
		mtdDevice.getDevice().destroyAccelerationStructureKHR(accelerationStructure, nullptr, mtdDevice.getDLDI());
	resourceManager.deleteResource(bufferID);
}

mtd::RayTracingTlas::RayTracingTlas(RayTracingTlas&& other) noexcept
	: mtdDevice{other.mtdDevice}, resourceManager{other.resourceManager},
	accelerationStructure{other.accelerationStructure}, bufferID{other.bufferID}
{
	other.accelerationStructure = nullptr;
	other.bufferID = 0U;
}

void mtd::RayTracingTlas::create
(
	const std::vector<SceneInstance> sceneInstances,
	const std::vector<RayTracingBlas>& blasList,
	const CommandHandler& commandHandler
)
{
	assert
	(
		accelerationStructure == nullptr
		&& "A TLAS cannot be created over and already initialized acceleration structure."
	);
	assert
	(
		!blasList.empty() && !sceneInstances.empty()
		&& "There must be at least one BLAS and one scene instance to create the TLAS."
	);

	ResourceID instanceBufferID = createInstanceBuffer(sceneInstances, blasList, commandHandler);

	vk::AccelerationStructureGeometryInstancesDataKHR tlasGeometryInstancesData{};
	tlasGeometryInstancesData.arrayOfPointers = vk::False;
	tlasGeometryInstancesData.data.deviceAddress = resourceManager.getBufferAddress(instanceBufferID);
	tlasGeometryInstancesData.pNext = nullptr;

	vk::AccelerationStructureGeometryKHR geometry{};
	geometry.geometryType = vk::GeometryTypeKHR::eInstances;
	geometry.geometry.instances = tlasGeometryInstancesData;
	geometry.flags = vk::GeometryFlagsKHR();
	geometry.pNext = nullptr;

	createAndBuild(commandHandler, geometry, sceneInstances.size());

	resourceManager.deleteResource(instanceBufferID);
	resourceManager.addAccelerationStructureReference(accelerationStructure, bufferID);
}

void mtd::RayTracingTlas::clear()
{
	if(accelerationStructure)
	{
		mtdDevice.getDevice().destroyAccelerationStructureKHR(accelerationStructure, nullptr, mtdDevice.getDLDI());
		accelerationStructure = nullptr;
	}
	resourceManager.deleteResource(bufferID);
	bufferID = 0U;
}

mtd::ResourceID mtd::RayTracingTlas::createInstanceBuffer
(
	const std::vector<SceneInstance> sceneInstances,
	const std::vector<RayTracingBlas>& blasList,
	const CommandHandler& commandHandler
) const
{
	assert
	(
		!blasList.empty() && !sceneInstances.empty()
		&& "There must be at least one BLAS and one scene instance to create the instance buffer."
	);

	std::vector<vk::AccelerationStructureInstanceKHR> asInstances(sceneInstances.size());
	for(size_t i = 0; i < sceneInstances.size(); i++)
	{
		assert(sceneInstances[i].meshID < blasList.size() && "Mesh ID must be within the range of mesh count.");

		asInstances[i].transform.matrix[0][0] = sceneInstances[i].transform.x.x;
		asInstances[i].transform.matrix[0][1] = sceneInstances[i].transform.y.x;
		asInstances[i].transform.matrix[0][2] = sceneInstances[i].transform.z.x;
		asInstances[i].transform.matrix[0][3] = sceneInstances[i].transform.w.x;

		asInstances[i].transform.matrix[1][0] = sceneInstances[i].transform.x.y;
		asInstances[i].transform.matrix[1][1] = sceneInstances[i].transform.y.y;
		asInstances[i].transform.matrix[1][2] = sceneInstances[i].transform.z.y;
		asInstances[i].transform.matrix[1][3] = sceneInstances[i].transform.w.y;

		asInstances[i].transform.matrix[2][0] = sceneInstances[i].transform.x.z;
		asInstances[i].transform.matrix[2][1] = sceneInstances[i].transform.y.z;
		asInstances[i].transform.matrix[2][2] = sceneInstances[i].transform.z.z;
		asInstances[i].transform.matrix[2][3] = sceneInstances[i].transform.w.z;

		asInstances[i].instanceCustomIndex = static_cast<uint32_t>(i);
		asInstances[i].mask = 0xFF;
		asInstances[i].instanceShaderBindingTableRecordOffset = 0U;
		asInstances[i].flags
			= (VkGeometryInstanceFlagBitsKHR)vk::GeometryInstanceFlagBitsKHR::eTriangleFacingCullDisable;
		asInstances[i].accelerationStructureReference = blasList[sceneInstances[i].meshID].getBufferAddress();
	}

	ResourceID instanceBufferID = resourceManager.createBuffer
	(
		"", GpuBufferType::AccelerationStructureInput, GpuMemoryUsage::CpuUpload,
		asInstances.size() * sizeof(vk::AccelerationStructureInstanceKHR), asInstances.data()
	);

	return instanceBufferID;
}

void mtd::RayTracingTlas::createAndBuild
(
	const CommandHandler& commandHandler,
	const vk::AccelerationStructureGeometryKHR& geometry,
	uint32_t instanceCount
)
{
	vk::AccelerationStructureBuildGeometryInfoKHR buildGeometryInfo;
	buildGeometryInfo.type = vk::AccelerationStructureTypeKHR::eTopLevel;
	buildGeometryInfo.flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace;
	buildGeometryInfo.mode = vk::BuildAccelerationStructureModeKHR::eBuild;
	buildGeometryInfo.srcAccelerationStructure = nullptr;
	buildGeometryInfo.dstAccelerationStructure = nullptr;
	buildGeometryInfo.geometryCount = 1U;
	buildGeometryInfo.pGeometries = &geometry;
	buildGeometryInfo.ppGeometries = nullptr;

	vk::AccelerationStructureBuildSizesInfoKHR buildSizesInfo{};
	mtdDevice.getDevice().getAccelerationStructureBuildSizesKHR
	(
		vk::AccelerationStructureBuildTypeKHR::eDevice, &buildGeometryInfo,
		&instanceCount, &buildSizesInfo, mtdDevice.getDLDI()
	);
	bufferID = resourceManager.createBuffer
	(
		"TLAS", GpuBufferType::AccelerationStructure, GpuMemoryUsage::GpuOnly,
		buildSizesInfo.accelerationStructureSize
	);

	vk::AccelerationStructureCreateInfoKHR asCreateInfo{};
	asCreateInfo.createFlags = vk::AccelerationStructureCreateFlagsKHR();
	asCreateInfo.buffer = resourceManager.getVulkanBuffer(bufferID);
	asCreateInfo.offset = 0UL;
	asCreateInfo.size = buildSizesInfo.accelerationStructureSize;
	asCreateInfo.type = vk::AccelerationStructureTypeKHR::eTopLevel;
	asCreateInfo.deviceAddress = 0UL;

	vk::Result result = mtdDevice.getDevice()
		.createAccelerationStructureKHR(&asCreateInfo, nullptr, &accelerationStructure, mtdDevice.getDLDI());
	if(result != vk::Result::eSuccess)
		LOG_ERROR("Failed to create top level acceleration structure. Vulkan result: %d", result);

	ResourceID scratchBufferID = resourceManager.createBuffer
	(
		"", GpuBufferType::Storage | GpuBufferType::DeviceAddress,
		GpuMemoryUsage::GpuOnly, buildSizesInfo.buildScratchSize
	);

	buildGeometryInfo.dstAccelerationStructure = accelerationStructure;
	buildGeometryInfo.scratchData.deviceAddress = resourceManager.getBufferAddress(scratchBufferID);

	vk::AccelerationStructureBuildRangeInfoKHR buildRangeInfo{};
	buildRangeInfo.primitiveCount = instanceCount;
	buildRangeInfo.primitiveOffset = 0U;
	buildRangeInfo.firstVertex = 0U;
	buildRangeInfo.transformOffset = 0U;

	const vk::AccelerationStructureBuildRangeInfoKHR* pBuildRangeInfo = &buildRangeInfo;

	vk::CommandBuffer commandBuffer = commandHandler.beginSingleTimeCommand();
	commandBuffer.buildAccelerationStructuresKHR(1U, &buildGeometryInfo, &pBuildRangeInfo, mtdDevice.getDLDI());
	commandHandler.endSingleTimeCommand(commandBuffer);

	resourceManager.deleteResource(scratchBufferID);
}
