#pragma once

#include "RayTracingTlas.hpp"

namespace mtd
{
	// Bounding Volume Hierarchy structure for the ray tracing pipeline
	class RayTracingBVH
	{
		public:
			RayTracingBVH(const Device& mtdDevice, ResourceManager& resourceManager);
			~RayTracingBVH() = default;

			RayTracingBVH(const RayTracingBVH&) = delete;
			RayTracingBVH& operator=(const RayTracingBVH&) = delete;

			RayTracingBVH(RayTracingBVH&& other) noexcept;

			// Builds the BLAS list and TLAS based on the scene data
			void createAccelerationStructures
			(
				const Device& mtdDevice, ResourceManager& resourceManager,
				const std::vector<MeshData>& meshes, const std::vector<SceneInstance> instances
			);
			// Destroys all acceleration structures
			void clear();

		private:
			// Top-Level Acceleration Structure
			RayTracingTlas tlas;
			// Bottom-Level Acceleration Structures
			std::vector<RayTracingBlas> blasList;

			// BVH command handler
			CommandHandler commandHandler;

			// Resource manager reference
			const ResourceManager& resourceManager;
	};
}
