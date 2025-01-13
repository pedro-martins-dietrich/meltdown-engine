#pragma once

#include "../Vulkan/Image/Texture.hpp"

namespace mtd
{
	// Bundle with all textures and values to represent a mesh material
	class Material
	{
		public:
			Material(const MaterialInfo& materialInfo);
			~Material() = default;

			Material(const Material&) = delete;
			Material& operator=(const Material&) = delete;

			Material(Material&& other) noexcept;

			// Getter
			uint32_t getTextureCount() const { return static_cast<uint32_t>(textureTypes.size()); }

			// Checks if the material has float data attributes
			bool hasFloatData() const { return !floatAttributeTypes.empty(); }

			// Inserts a float attribute data
			void addFloatData(MaterialFloatDataType floatDataType, const float* data);
			// Adds a texture path to load the material
			void addTexturePath(MaterialTextureType textureType, std::string&& texturePath);

			// Loads all float attributes and textures to the GPU
			void loadMaterial
			(
				const Device& device,
				const CommandHandler& commandHandler,
				DescriptorSetHandler& descriptorSetHandler,
				uint32_t swappableSetIndex
			);

		private:
			// Lists identifying which material attributes are used and in which order
			std::vector<MaterialFloatDataType> floatAttributeTypes;
			std::vector<MaterialTextureType> textureTypes;

			// Offsets for each float attribute
			std::unordered_map<MaterialFloatDataType, uint32_t> floatAttributeOffsets;
			// Texture file paths
			std::unordered_map<MaterialTextureType, std::string> texturePaths;

			// Lists of float attributes and texture attributes
			std::vector<float> floatAttributes;
			std::vector<Texture> textures;

			// Loads all float attributes to the GPU
			void loadFloatData
			(
				const Device& device,
				DescriptorSetHandler& descriptorSetHandler,
				uint32_t swappableSetIndex
			) const;
			// Loads all material textures
			void loadTextures
			(
				const Device& device,
				const CommandHandler& commandHandler,
				DescriptorSetHandler& descriptorSetHandler,
				uint32_t swappableSetIndex
			);

			// Finds the dimension (amount of floats) of each material float data type
			uint32_t mapFloatDataTypeDimension(MaterialFloatDataType floatDataType) const;
	};
}
