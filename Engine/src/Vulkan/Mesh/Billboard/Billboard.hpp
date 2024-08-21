#pragma once

#include <memory>

#include "../Mesh.hpp"
#include "../../Image/Texture.hpp"

namespace mtd
{
	// Stores billboard data
	class Billboard : public Mesh
	{
		public:
			Billboard(uint32_t id, const char* texturePath, glm::mat4 preTransform = glm::mat4{1.0f});
			~Billboard() = default;

			Billboard(const Billboard&) = delete;
			Billboard& operator=(const Billboard&) = delete;

			Billboard(Billboard&& other) noexcept;

			// Loads the billboard texture
			void loadTexture
			(
				const Device& device,
				const CommandHandler& commandHandler,
				DescriptorSetHandler& descriptorSetHandler
			);

		private:
			// Billboard ID
			uint32_t id;

			// Bilboard texture
			std::string texturePath;
			std::unique_ptr<Texture> texture;
	};
}
