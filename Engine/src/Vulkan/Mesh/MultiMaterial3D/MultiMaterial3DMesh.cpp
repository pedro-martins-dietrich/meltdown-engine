#include <pch.hpp>
#include "MultiMaterial3DMesh.hpp"

#include "../ObjMeshLoader.hpp"

mtd::MultiMaterial3DMesh::MultiMaterial3DMesh
(
	const Device& device,
	uint32_t index,
	const char* id,
	const char* fileName,
	const MaterialInfo& materialInfo,
	const std::vector<Mat4x4>& preTransforms
) : Mesh{device, index, id, preTransforms, 1}, nextMeshIndexOffset{0}
{
	ObjMeshLoader::loadMultiMaterial3DMesh(fileName, vertices, indices, submeshInfos, materials, materialInfo);
}

mtd::MultiMaterial3DMesh::MultiMaterial3DMesh(MultiMaterial3DMesh&& other) noexcept
	: Mesh{std::move(other)},
	vertices{std::move(other.vertices)},
	indices{std::move(other.indices)},
	materials{std::move(other.materials)},
	submeshInfos{std::move(other.submeshInfos)},
	nextMeshIndexOffset{other.nextMeshIndexOffset}
{
}

uint32_t mtd::MultiMaterial3DMesh::getTextureCount() const
{
	if(materials.empty()) return 0;
	return static_cast<uint32_t>(materials[0].getTextureCount() * materials.size());
}

uint32_t mtd::MultiMaterial3DMesh::getSubmeshIndexCount(uint32_t submeshIndex) const
{
	if(submeshIndex + 1 < submeshInfos.size())
		return submeshInfos[submeshIndex + 1].indexOffset - submeshInfos[submeshIndex].indexOffset;
	if(submeshIndex + 1 == submeshInfos.size())
		return nextMeshIndexOffset - submeshInfos[submeshIndex].indexOffset;
	return 0;
}

// Sets all sub-mesh index offsets in the lump
void mtd::MultiMaterial3DMesh::setIndexOffset(uint32_t offset)
{
	for(SubmeshData& submeshInfo: submeshInfos)
		submeshInfo.indexOffset += offset;

	nextMeshIndexOffset = indices.size() + offset;
}

// Checks if the used material has float data attributes
bool mtd::MultiMaterial3DMesh::hasMaterialFloatData() const
{
	if(materials.empty()) return false;
	return materials[0].hasFloatData();
}

// Loads mesh materials
void mtd::MultiMaterial3DMesh::loadMaterials
(
	const Device& device,
	const CommandHandler& commandHandler,
	DescriptorSetHandler& descriptorSetHandler,
	uint32_t initialMaterialIndex
)
{
	for(uint32_t i = 0; i < materials.size(); i++)
		materials[i].loadMaterial(device, commandHandler, descriptorSetHandler, initialMaterialIndex + i);
}
