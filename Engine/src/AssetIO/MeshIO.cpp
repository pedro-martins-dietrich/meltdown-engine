#include <pch.hpp>
#include "MeshIO.hpp"

#include "AssetFileHeaders.hpp"
#include "../Utils/FileHandler.hpp"
#include "../Utils/Logger.hpp"
#include "../Utils/StringParser.hpp"

namespace mtd
{
	constexpr uint64_t MESH_MAGIC = "MTD_MESH"_u64;
	constexpr uint32_t MESH_FILE_VERSION = 1U;
}

bool mtd::MeshIO::saveMesh
(
	std::string_view fileName,
	const std::vector<Vertex>& vertices,
	const std::vector<uint32_t>& indices,
	const std::vector<SubmeshData>& submeshData
)
{
	std::string meshPath{MTD_RESOURCES_PATH};
	meshPath.append("meshes/");
	meshPath.append(fileName);
	meshPath.append(".mesh");

	std::ofstream meshFile{meshPath, std::ios::binary};
	if(!meshFile)
	{
		LOG_WARNING("Failed to save mesh to \"%s\".", meshPath.c_str());
		return false;
	}

	MeshHeader header{};
	header.magic = MESH_MAGIC;
	header.version = MESH_FILE_VERSION;
	header.vertexType = 0;
	header.vertexCount = static_cast<uint32_t>(vertices.size());
	header.indexCount = static_cast<uint32_t>(indices.size());
	header.submeshCount = static_cast<uint32_t>(submeshData.size());

	size_t offset = sizeof(MeshHeader);
	size_t vertexDataSize = vertices.size() * sizeof(Vertex);
	size_t indexDataSize = indices.size() * sizeof(uint32_t);
	size_t submeshDataSize = submeshData.size() * sizeof(SubmeshData);

	header.vertexDataOffset = static_cast<uint32_t>(offset);
	offset += vertexDataSize;
	header.indexDataOffset = static_cast<uint32_t>(offset);
	offset += indexDataSize;
	header.submeshDataOffset = static_cast<uint32_t>(offset);

	header.flags = 0U;

	meshFile.write(reinterpret_cast<char*>(&header), sizeof(MeshHeader));
	meshFile.write(reinterpret_cast<const char*>(vertices.data()), vertexDataSize);
	meshFile.write(reinterpret_cast<const char*>(indices.data()), indexDataSize);
	meshFile.write(reinterpret_cast<const char*>(submeshData.data()), submeshDataSize);

	meshFile.close();

	LOG_INFO("Mesh saved to \"%s\".", meshPath.c_str());

	return true;
}

bool mtd::MeshIO::loadMesh
(
	std::string_view fileName,
	std::vector<Vertex>& vertices,
	std::vector<uint32_t>& indices,
	std::vector<SubmeshData>& submeshData
)
{
	vertices.clear();
	indices.clear();
	submeshData.clear();

	std::string meshPath{MTD_RESOURCES_PATH};
	meshPath.append("meshes/");
	meshPath.append(fileName);
	meshPath.append(".mesh");

	std::ifstream meshFile{meshPath, std::ios::binary | std::ios::ate};
	if(!meshFile)
	{
		LOG_WARNING("Failed to find mesh file \"%s\" for loading.", meshPath.c_str());
		return false;
	}

	std::streamsize meshFileSize = meshFile.tellg();
	if(meshFileSize < static_cast<std::streamsize>(sizeof(MeshHeader)))
	{
		LOG_WARNING("Invalid header for mesh file \"%s\".", meshPath.c_str());
		return false;
	}

	MeshHeader header{};
	meshFile.seekg(0, std::ios::beg);
	meshFile.read(reinterpret_cast<char*>(&header), sizeof(MeshHeader));

	bool validMeshFile = true;
	validMeshFile &= header.magic == MESH_MAGIC;
	validMeshFile &= header.version == MESH_FILE_VERSION;
	validMeshFile &= header.vertexType == 0;
	validMeshFile &= header.vertexDataOffset <= static_cast<uint32_t>(meshFileSize);
	validMeshFile &= header.indexDataOffset <= static_cast<uint32_t>(meshFileSize);
	validMeshFile &= header.submeshDataOffset <= static_cast<uint32_t>(meshFileSize);

	size_t vertexDataSize = header.vertexCount * sizeof(Vertex);
	size_t indexDataSize = header.indexCount * sizeof(uint32_t);
	size_t submeshDataSize = header.submeshCount * sizeof(SubmeshData);

	validMeshFile &= (header.vertexDataOffset + vertexDataSize) <= static_cast<size_t>(meshFileSize);
	validMeshFile &= (header.indexDataOffset + indexDataSize) <= static_cast<size_t>(meshFileSize);
	validMeshFile &= (header.submeshDataOffset + submeshDataSize) <= static_cast<size_t>(meshFileSize);

	if(!validMeshFile)
	{
		LOG_WARNING("Invalid mesh file \"%s\".", meshPath.c_str());
		return false;
	}

	vertices.resize(header.vertexCount);
	indices.resize(header.indexCount);
	submeshData.resize(header.submeshCount);

	meshFile.seekg(header.vertexDataOffset, std::ios::beg);
	meshFile.read(reinterpret_cast<char*>(vertices.data()), vertexDataSize);
	meshFile.seekg(header.indexDataOffset, std::ios::beg);
	meshFile.read(reinterpret_cast<char*>(indices.data()), indexDataSize);
	meshFile.seekg(header.submeshDataOffset, std::ios::beg);
	meshFile.read(reinterpret_cast<char*>(submeshData.data()), submeshDataSize);

	meshFile.close();

	LOG_INFO("Mesh loaded from \"%s\".", meshPath.c_str());

	return true;
}
