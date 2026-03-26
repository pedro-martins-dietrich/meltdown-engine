#include <pch.hpp>
#include "DefaultMesh.hpp"

#include "../../../AssetIO/MeshIO.hpp"
#include "../../../AssetIO/MaterialIO.hpp"
#include "../../../Utils/StringParser.hpp"

mtd::DefaultMesh::DefaultMesh
(
	const Device& device,
	uint32_t index,
	const char* id,
	const char* fileName,
	const MaterialInfo& materialInfo,
	const std::vector<Mat4x4>& preTransforms
) : Mesh{device, index, id, preTransforms, 1}, material{materialInfo}, indexOffset{0}
{
	std::string fileStem = StringParser::getFileStem(fileName, false);
	std::vector<SubmeshData> submeshesInfo{};

	bool loadOk = MeshIO::loadMesh(fileStem, vertices, indices, submeshesInfo);
	loadOk &= MaterialIO::loadMaterial(fileStem, material);
	if(!loadOk)
		throw std::runtime_error{"Failed to create mesh \"" + fileStem + "\"."};
}

mtd::DefaultMesh::DefaultMesh(DefaultMesh&& other) noexcept
	: Mesh{std::move(other)},
	vertices{std::move(other.vertices)},
	indices{std::move(other.indices)},
	material{std::move(other.material)},
	indexOffset{other.indexOffset}
{}

void mtd::DefaultMesh::loadTexture
(
	const Device& device, const CommandHandler& commandHandler, DescriptorSetHandler& descriptorSetHandler
)
{
	material.loadMaterial(device, commandHandler, descriptorSetHandler, meshIndex);
}
