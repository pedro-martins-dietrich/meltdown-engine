#pragma once

#include <memory>

#include "Billboard.hpp"
#include "../MeshManager.hpp"
#include "../../Image/Texture.hpp"

namespace mtd
{
	// Handles data from all billboards
	class BillboardManager : public MeshManager
	{
		public:
			BillboardManager(const Device& device);
			~BillboardManager();

			BillboardManager(const BillboardManager&) = delete;
			BillboardManager& operator=(const BillboardManager&) = delete;

			// Getters
			virtual uint32_t getMeshCount() const override
				{ return static_cast<uint32_t>(billboards.size()); }
			std::vector<Billboard>& getBillboards() { return billboards; }

			// Loads the billboards textures to the GPU
			virtual void loadMeshes
			(
				const CommandHandler& commandHandler,
				DescriptorSetHandler& textureDescriptorSetHandler
			) override;

			// Clears the list of billboards and the instance buffer
			virtual void clearMeshes() override;

			// Executes the start code for each model on scene loading
			virtual void start() override;
			// Updates instances data
			virtual void update(double frameTime) override;

			// There is no buffer common to all billboards to be binded
			virtual void bindBuffers(const vk::CommandBuffer& commandBuffer) const override;
			// Draws the mesh specified by the index
			virtual void drawMesh
			(
				const vk::CommandBuffer& commandBuffer, uint32_t meshIndex
			) const override;

		private:
			// List of billboards
			std::vector<Billboard> billboards;
			// Map to translate a model ID to a mesh index
			std::unordered_map<std::string, uint32_t> billboardIndexMap;

			// Textures for each billboard
			std::vector<std::string> texturePaths;
			std::vector<std::unique_ptr<Texture>> textures;

			// Device reference
			const Device& device;
	};
}
