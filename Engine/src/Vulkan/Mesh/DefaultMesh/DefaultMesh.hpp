#pragma once

#include <memory>

#include "../Mesh.hpp"
#include "../../Image/Texture.hpp"
#include "../../../Utils/EngineStructs.hpp"

namespace mtd
{
	// Stores default mesh data
	class DefaultMesh : public Mesh
	{
		public:
			DefaultMesh(uint32_t id, const char* fileName, glm::mat4 preTransform = glm::mat4{1.0f});
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
			// Mesh ID
			uint32_t id;

			// Mesh data
			std::vector<Vertex> vertices;
			std::vector<uint32_t> indices;
			uint32_t indexOffset;
			
			// Mesh texture
			std::string diffuseTexturePath;
			std::unique_ptr<Texture> diffuseTexture;
	};
}
