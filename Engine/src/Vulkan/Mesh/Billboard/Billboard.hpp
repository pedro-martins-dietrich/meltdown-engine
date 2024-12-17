#pragma once

#include "../Mesh.hpp"
#include "../../../Material/Material.hpp"

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
				const MaterialInfo& materialInfo,
				const std::vector<Mat4x4>& preTransforms
			);
			~Billboard() = default;

			Billboard(Billboard&& other) noexcept;

			// Getter
			uint32_t getTextureCount() const { return material.getTextureCount(); }

			// Checks if the used material has float data attributes
			bool hasMaterialFloatData() const { return material.hasFloatData(); }

			// Loads the billboard texture
			void loadTexture(const CommandHandler& commandHandler, DescriptorSetHandler& descriptorSetHandler);

		private:
			// Bilboard material
			Material material;
	};
}
