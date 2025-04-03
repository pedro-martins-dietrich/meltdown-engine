#pragma once

#include "../Vulkan/Image/Texture.hpp"

namespace mtd
{
	// Manages all materials of the same mesh manager
	class MaterialLump
	{
		public:
			MaterialLump(const mtd::Device& mtdDevice, const MaterialInfo& materialInfo);
			~MaterialLump() = default;

			MaterialLump(const MaterialLump&) = delete;
			MaterialLump& operator=(const MaterialLump&) = delete;

			// Getters
			const MaterialInfo& getMaterialInfo() const { return materialInfo; }
			uint32_t getMaterialCount() const { return materialCount; }
			uint32_t getTextureCount() const { return static_cast<uint32_t>(materialTextures.size()); }

			// Checks if the material type has float data attributes
			bool hasFloatData() const { return floatMaterialAttributes.size() != 0; }

			// Loads material float data and textures
			void addMaterial(const float* data, size_t size);

			// Assigns the material buffer as a descriptor
			void assignBufferToDescriptor
			(
				DescriptorSetHandler& descriptorSetHandler, uint32_t swappableSetIndex, uint32_t bindingIndex
			) const;

			// Loads all materials to the GPU VRAM
			void loadMaterialsToGPU(const CommandHandler& commandHandler);

		private:
			// Information about the material data
			MaterialInfo materialInfo;

			// Buffer data for float attributes of all materials
			std::vector<float> floatMaterialAttributes;
			// List for the texture of all materials
			std::vector<Texture> materialTextures;

			// Buffers
			GpuBuffer floatDataBuffer;

			// Counter for the materials in the lump
			uint32_t materialCount;
	};
}
