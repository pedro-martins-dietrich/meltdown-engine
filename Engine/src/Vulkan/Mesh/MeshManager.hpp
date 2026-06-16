#pragma once

#include "../Descriptors/DescriptorSetHandler.hpp"

namespace mtd
{
	// Interface for the mesh managers
	class MeshManager
	{
		public:
			MeshManager(const Device& device);
			virtual ~MeshManager() = default;

			MeshManager(const MeshManager&) = delete;
			MeshManager& operator=(const MeshManager&) = delete;

			// Getters
			virtual uint32_t getMeshCount() const = 0;
			virtual uint32_t getMaterialCount() const = 0;
			virtual uint32_t getTextureCount() const = 0;

			// Checks if the material type for the stored meshes has float data
			virtual bool hasMaterialFloatData() const = 0;

			// Loads mesh data to the GPU
			virtual void loadMeshes(DescriptorSetHandler& textureDescriptorSetHandler) = 0;

			// Clears mesh data
			virtual void clearMeshes() = 0;

			// Executes the start code for each mesh on scene loading
			virtual void start() = 0;
			// Updates mesh data
			virtual void update(double frameTime) = 0;

		protected:
			// Mesh manager command handler
			CommandHandler commandHandler;

			// Device reference
			const Device& device;
	};
}
