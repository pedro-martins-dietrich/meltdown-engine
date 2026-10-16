#include <pch.hpp>
#include "RayTracingBVH.hpp"

mtd::RayTracingBVH::RayTracingBVH(const Device& mtdDevice, ResourceManager& resourceManager)
	: resourceManager{resourceManager}, commandHandler{mtdDevice}, tlas{mtdDevice, resourceManager}
{}

mtd::RayTracingBVH::RayTracingBVH(RayTracingBVH&& other) noexcept
	: resourceManager{other.resourceManager},
	tlas{std::move(other.tlas)}, blasList{std::move(other.blasList)},
	commandHandler{std::move(other.commandHandler)}
{}

void mtd::RayTracingBVH::createAccelerationStructures
(
	const Device& mtdDevice, ResourceManager& resourceManager,
	const std::vector<MeshData>& meshes, const std::vector<SceneInstance> instances
)
{
	const MeshData& mesh = meshes.back();

	for(const MeshData& meshData: meshes)
		blasList.emplace_back(mtdDevice, resourceManager, commandHandler, meshData);
	tlas.create(instances, blasList, commandHandler);
}

void mtd::RayTracingBVH::clear()
{
	tlas.clear();
	blasList.clear();
}
