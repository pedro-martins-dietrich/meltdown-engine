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
			Billboard
			(
				const Device& device,
				uint32_t index,
				const char* id,
				const char* texturePath,
				const Mat4x4& preTransform = Mat4x4{1.0f}
			);
			~Billboard() = default;

			Billboard(const Billboard&) = delete;
			Billboard& operator=(const Billboard&) = delete;

			Billboard(Billboard&& other) noexcept;

			// Loads the billboard texture
			void loadTexture
			(
				const CommandHandler& commandHandler, DescriptorSetHandler& descriptorSetHandler
			);

		private:
			// Bilboard texture
			std::string texturePath;
			std::unique_ptr<Texture> texture;
	};
}
