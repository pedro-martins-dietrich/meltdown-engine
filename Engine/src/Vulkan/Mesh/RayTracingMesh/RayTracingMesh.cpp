#include <pch.hpp>
#include "RayTracingMesh.hpp"

#include "../ObjMeshLoader.hpp"
#include "../../../AssetIO/MeshIO.hpp"
#include "../../../AssetIO/MaterialIO.hpp"
#include "../../../Utils/StringParser.hpp"

mtd::RayTracingMesh::RayTracingMesh
(
	const Device& device,
	uint32_t index,
	const char* id,
	const char* fileName,
	MaterialLump& materialLump,
	const std::vector<Mat4x4>& preTransforms
) : Mesh{device, index, id, preTransforms, 1}, materialCount{0U}
{
	std::vector<Material> materials;
	std::string fileStem = StringParser::getFileStem(fileName, false);

	ObjMeshLoader::loadRayTracingMesh
	(
		fileName, vertices, indices, materialIndices, materials, materialLump.getMaterialInfo()
	);
	materialCount = static_cast<uint32_t>(materials.size());

	for(int i = 0; i < materials.size(); i++)
	{
		std::vector<std::string> texturePaths;
		materials[i].fetchTexturePaths(texturePaths);
		materialLump.addMaterial
		(
			materials[i].getFloatAttributesData(), materials[i].getFloatAttributesSize(), texturePaths
		);

		// Each material must have its own name, and save to a different path
		if(!MaterialIO::saveMaterial(fileStem, materials[i]))
			throw std::runtime_error{"Failed to save material for mesh \"" + fileStem + "\"."};
	}
	
	//std::vector<SubmeshData> submeshData{SubmeshData{static_cast<uint32_t>(indices.size()), 0, 0}};
	//MeshIO::saveMesh(fileStem, vertices, indices, submeshData);

	//bool loadOk = MeshIO::loadMesh(fileStem, vertices, indices, submeshData);
}

mtd::RayTracingMesh::RayTracingMesh(RayTracingMesh&& other) noexcept
	: Mesh{std::move(other)},
	vertices{std::move(other.vertices)},
	indices{std::move(other.indices)},
	materialIndices{std::move(other.materialIndices)},
	materialCount{other.materialCount}
{}

void mtd::RayTracingMesh::clearMeshData()
{
	vertices.clear();
	indices.clear();
	materialIndices.clear();
}
