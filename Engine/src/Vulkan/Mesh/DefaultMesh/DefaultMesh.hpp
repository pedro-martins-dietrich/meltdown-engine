#pragma once

#include "../Mesh.hpp"
#include "../../Image/Texture.hpp"
#include "../../../Utils/EngineStructs.hpp"

namespace mtd
{
	// Stores default mesh data
	class DefaultMesh : public Mesh
	{
		public:
			DefaultMesh
			(
				const Device& device,
				uint32_t index,
				const char* id,
				const char* fileName,
				const std::vector<Mat4x4>& preTransforms
			);
			~DefaultMesh() = default;

			DefaultMesh(const DefaultMesh&) = delete;
			DefaultMesh& operator=(const DefaultMesh&) = delete;

			DefaultMesh(DefaultMesh&& other) noexcept;

			// Getters
			const std::vector<Vertex>& getVertices() const { return vertices; }
			const std::vector<uint32_t>& getIndices() const { return indices; }
			uint32_t getIndexOffset() const { return indexOffset; }

			// Sets index offset in the lump
			void setIndexOffset(uint32_t offset) { indexOffset = offset; }

			// Loads mesh texture
			void loadTexture
			(
				const Device& device,
				const CommandHandler& commandHandler,
				DescriptorSetHandler& descriptorSetHandler
			);

		private:
			// Mesh data
			std::vector<Vertex> vertices;
			std::vector<uint32_t> indices;
			uint32_t indexOffset;
			
			// Mesh texture
			std::string diffuseTexturePath;
			std::unique_ptr<Texture> diffuseTexture;
	};
}
