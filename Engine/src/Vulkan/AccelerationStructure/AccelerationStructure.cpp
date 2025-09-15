#include <pch.hpp>
#include "AccelerationStructure.hpp"

#include "ScratchAccelerationStructure.hpp"
#include "../../Utils/Logger.hpp"

mtd::AccelerationStructure::AccelerationStructure(const Device& device)
	: device{device},
	accelerationStructure{nullptr},
	asBuffer
	{
		device,
		vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress,
		vk::MemoryPropertyFlagBits::eDeviceLocal
	},
	asType{vk::AccelerationStructureTypeKHR::eGeneric},
	geometry{},
	asBuildGeometryInfo{}
{}

mtd::AccelerationStructure::~AccelerationStructure()
{
	device.getDevice().destroyAccelerationStructureKHR(accelerationStructure, nullptr, device.getDLDI());
}

// Creates the Bottom Level Acceleration Structure (BLAS)
void mtd::AccelerationStructure::createBLAS
(
	const CommandHandler& commandHandler,
	vk::DeviceAddress vertexBufferAddress,
	uint32_t vertexCount,
	vk::DeviceAddress indexBufferAddress,
	uint32_t triangleCount
)
{
	assert
	(
		accelerationStructure == nullptr
		&& "A BLAS cannot be created over and already initialized acceleration structure."
	);

	asType = vk::AccelerationStructureTypeKHR::eBottomLevel;

	vk::AccelerationStructureGeometryTrianglesDataKHR asGeometryTrianglesData{};
	asGeometryTrianglesData.vertexFormat = vk::Format::eR32G32B32Sfloat;
	asGeometryTrianglesData.vertexData.deviceAddress = vertexBufferAddress;
	asGeometryTrianglesData.vertexStride = sizeof(Vertex);
	asGeometryTrianglesData.maxVertex = vertexCount;
	asGeometryTrianglesData.indexType = vk::IndexType::eUint32;
	asGeometryTrianglesData.indexData.deviceAddress = indexBufferAddress;
	asGeometryTrianglesData.transformData.deviceAddress = 0;

	geometry.geometryType = vk::GeometryTypeKHR::eTriangles;
	geometry.geometry.triangles = asGeometryTrianglesData;
	geometry.flags = vk::GeometryFlagBitsKHR::eOpaque;

	createAS(commandHandler, triangleCount);
}

// Creates the Top Level Acceleration Structure (TLAS)
void mtd::AccelerationStructure::createTLAS
(
	const CommandHandler& commandHandler, vk::DeviceAddress instanceBufferAddress
)
{
	assert
	(
		accelerationStructure == nullptr
		&& "A TLAS cannot be created over and already initialized acceleration structure."
	);
	
	asType = vk::AccelerationStructureTypeKHR::eTopLevel;

	vk::AccelerationStructureGeometryInstancesDataKHR tlasGeometryInstancesData{};
	tlasGeometryInstancesData.arrayOfPointers = vk::False;
	tlasGeometryInstancesData.data.deviceAddress = instanceBufferAddress;

	geometry.geometryType = vk::GeometryTypeKHR::eInstances;
	geometry.geometry.instances = tlasGeometryInstancesData;

	createAS(commandHandler, 1U);
}

// Creates an instance buffer
mtd::GpuBuffer mtd::AccelerationStructure::createInstanceBuffer() const
{
	assert
	(
		accelerationStructure != nullptr && asType == vk::AccelerationStructureTypeKHR::eBottomLevel
		&& "The acceleration structure must be a bottom level type and needs to be created before the instance buffer."
	);

	vk::TransformMatrixKHR identityMatrix{};
	identityMatrix.matrix[0][0] = 1.0f;
	identityMatrix.matrix[1][1] = 1.0f;
	identityMatrix.matrix[2][2] = 1.0f;

	vk::AccelerationStructureInstanceKHR asInstance{};
	asInstance.transform = identityMatrix;
	asInstance.instanceCustomIndex = 0;
	asInstance.mask = 0xFF;
	asInstance.instanceShaderBindingTableRecordOffset = 0;
	asInstance.flags = (VkGeometryInstanceFlagBitsKHR)vk::GeometryInstanceFlagBitsKHR::eTriangleFacingCullDisable;
	asInstance.accelerationStructureReference = asBuffer.getBufferAddress();

	GpuBuffer instanceBuffer
	{
		device,
		sizeof(vk::AccelerationStructureInstanceKHR),
		vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR
		| vk::BufferUsageFlagBits::eShaderDeviceAddress,
		vk::MemoryPropertyFlagBits::eHostVisible
	};
	instanceBuffer.copyMemoryToBuffer(sizeof(vk::AccelerationStructureInstanceKHR), &asInstance);

	return instanceBuffer;
}

// Handles the common creation logic for both BLAS and TLAS
void mtd::AccelerationStructure::createAS(const CommandHandler& commandHandler, uint32_t primitiveCount)
{
	const vk::Device& vkDevice = device.getDevice();

	asBuildGeometryInfo.type = asType;
	asBuildGeometryInfo.flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace;
	asBuildGeometryInfo.mode = vk::BuildAccelerationStructureModeKHR::eBuild;
	asBuildGeometryInfo.srcAccelerationStructure = nullptr;
	asBuildGeometryInfo.dstAccelerationStructure = nullptr;
	asBuildGeometryInfo.geometryCount = 1;
	asBuildGeometryInfo.pGeometries = &geometry;
	asBuildGeometryInfo.ppGeometries = nullptr;

	vk::AccelerationStructureBuildSizesInfoKHR asBuildSizesInfo = vkDevice.getAccelerationStructureBuildSizesKHR
	(
		vk::AccelerationStructureBuildTypeKHR::eDevice, asBuildGeometryInfo, {primitiveCount}, device.getDLDI()
	);
	asBuffer.create(asBuildSizesInfo.accelerationStructureSize);

	vk::AccelerationStructureCreateInfoKHR asCreateInfo{};
	asCreateInfo.createFlags = vk::AccelerationStructureCreateFlagsKHR();
	asCreateInfo.buffer = asBuffer.getBuffer();
	asCreateInfo.offset = 0;
	asCreateInfo.size = asBuildSizesInfo.accelerationStructureSize;
	asCreateInfo.type = asType;
	asCreateInfo.deviceAddress = 0;

	vk::Result result =
		vkDevice.createAccelerationStructureKHR(&asCreateInfo, nullptr, &accelerationStructure, device.getDLDI());
	if(result != vk::Result::eSuccess)
		LOG_ERROR
		(
			"Failed to create %s level acceleration structure. Vulkan result: %d",
			asType == vk::AccelerationStructureTypeKHR::eBottomLevel ? "bottom" : "top",
			result
		);

	ScratchAccelerationStructure scratch{device, asBuildSizesInfo.buildScratchSize, asType};

	asBuildGeometryInfo.dstAccelerationStructure = accelerationStructure;
	asBuildGeometryInfo.scratchData.deviceAddress = scratch.getBufferAddress();

	buildAS(commandHandler, primitiveCount);
}

// Builds the acceleration structure in the GPU
void mtd::AccelerationStructure::buildAS(const CommandHandler& commandHandler, uint32_t primitiveCount) const
{
	vk::AccelerationStructureBuildRangeInfoKHR buildRangeInfo{};
	buildRangeInfo.primitiveCount = primitiveCount;
	const vk::AccelerationStructureBuildRangeInfoKHR* pBuildRangeInfo = &buildRangeInfo;

	vk::CommandBuffer commandBuffer = commandHandler.beginSingleTimeCommand();
	commandBuffer.buildAccelerationStructuresKHR
	(
		1, &asBuildGeometryInfo, &pBuildRangeInfo, device.getDLDI()
	);
	commandHandler.endSingleTimeCommand(commandBuffer);
}
