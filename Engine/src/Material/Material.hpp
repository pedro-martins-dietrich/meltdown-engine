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

			// Getters
			size_t getFloatAttributesSize() const { return floatAttributes.size(); }
			const float* getFloatAttributesData() const { return floatAttributes.data(); }
			uint32_t getTextureCount() const { return static_cast<uint32_t>(textureTypes.size()); }
			std::string_view getTexturePath(MaterialTextureType textureType) const
				{ return texturePaths.at(textureType); }
			const std::vector<MaterialFloatDataType>& getFloatAttributeTypes() const { return floatAttributeTypes; }
			const std::vector<MaterialTextureType>& getTextureTypes() const { return textureTypes; }
			std::vector<MaterialFloatDataType>& getFloatAttributeTypes() { return floatAttributeTypes; }
			std::vector<MaterialTextureType>& getTextureTypes() { return textureTypes; }
			std::vector<float>& getFloatAttributes() { return floatAttributes; }

			// Fetches the texture paths for the material
			void fetchTexturePaths(std::vector<std::string>& textureFilePaths) const;

			// Checks if the material has float data attributes
			bool hasFloatData() const { return !floatAttributeTypes.empty(); }

			// Inserts a float attribute data
			void addFloatData(MaterialFloatDataType floatDataType, const float* data);
			// Adds a texture path to load the material
			void addTexturePath(MaterialTextureType textureType, std::string&& texturePath);

			// Updates the float attribute offsets based on the material float data types
			void updateFloatAttributeOffsets();

			// Loads all float attributes and textures to the GPU
			void loadMaterial
			(
				const Device& device,
				const CommandHandler& commandHandler,
				DescriptorSetHandler& descriptorSetHandler,
				uint32_t swappableSetIndex,
				uint32_t bindingOffset = 0
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
				uint32_t swappableSetIndex,
				uint32_t bindingOffset
			) const;
			// Loads all material textures
			void loadTextures
			(
				const Device& device,
				const CommandHandler& commandHandler,
				DescriptorSetHandler& descriptorSetHandler,
				uint32_t swappableSetIndex,
				uint32_t bindingOffset
			);

			// Finds the dimension (amount of floats) of each material float data type
			uint32_t mapFloatDataTypeDimension(MaterialFloatDataType floatDataType) const;
	};
}
