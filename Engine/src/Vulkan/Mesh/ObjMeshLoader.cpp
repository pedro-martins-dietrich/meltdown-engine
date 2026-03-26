#include <pch.hpp>
#include "ObjMeshLoader.hpp"

#include "../../Utils/Logger.hpp"
#include "../../Utils/StringParser.hpp"

namespace mtd
{
	// Data bundle for the .obj loader
	struct ObjData
	{
		std::vector<Vertex>& vertices;
		std::vector<uint32_t>& indices;
		std::vector<Vec3> positions;
		std::vector<Vec2> textureCoordinates;
		std::vector<Vec3> normals;
		std::unordered_map<std::string, uint32_t> history;
	};

	// Loads a single material from a .mtl file
	static void loadMaterial(std::string filePath, Material& material);
	// Loads the materials from a .mtl file
	static void loadMaterials
	(
		std::string filePath,
		std::vector<Material>& materials,
		std::unordered_map<std::string, uint32_t>& materialIDs,
		const MaterialInfo& materialInfo
	);

	// Parses each triangle of a face, returning the number of triangles
	static size_t readFaceData(const std::vector<std::string>& words, ObjData& data);
	// Parses a vertex
	static void readVertex(const std::string& vertexDescription, ObjData& data);
}

void mtd::ObjMeshLoader::loadDefault3DMesh
(
	const char* fileName,
	std::vector<Vertex>& vertices,
	std::vector<uint32_t>& indices,
	Material& meshMaterial
)
{
	std::string objMeshPath{MTD_RESOURCES_PATH};
	objMeshPath.append("meshes/");
	objMeshPath.append(fileName);

	loadMaterial(objMeshPath, meshMaterial);

	std::string line;
	std::vector<std::string> words;

	ObjData data{vertices, indices};

	std::ifstream file;
	file.open(objMeshPath);
	while(std::getline(file, line))
	{
		StringParser::split(line, " ", words);

		if(!words[0].compare("v"))
			data.positions.emplace_back(std::stof(words[1]), std::stof(words[2]), std::stof(words[3]));
		else if(!words[0].compare("vt"))
			data.textureCoordinates.emplace_back(std::stof(words[1]), std::stof(words[2]));
		else if(!words[0].compare("vn"))
			data.normals.emplace_back(std::stof(words[1]), std::stof(words[2]), std::stof(words[3]));
		else if(!words[0].compare("f"))
			readFaceData(words, data);
	}
	file.close();

	LOG_VERBOSE("Mesh \"%s\" loaded.", fileName);
}

void mtd::ObjMeshLoader::loadMultiMaterial3DMesh
(
	const char* fileName,
	std::vector<Vertex>& vertices,
	std::vector<uint32_t>& indices,
	std::vector<SubmeshData>& submeshInfos,
	std::vector<Material>& meshMaterials,
	const MaterialInfo& materialInfo
)
{
	std::string objMeshPath{MTD_RESOURCES_PATH};
	objMeshPath.append("meshes/");
	objMeshPath.append(fileName);

	std::unordered_map<std::string, uint32_t> materialIDs;
	loadMaterials(objMeshPath, meshMaterials, materialIDs, materialInfo);

	std::string line;
	std::vector<std::string> words;

	ObjData data{vertices, indices};

	std::ifstream file;
	file.open(objMeshPath);
	while(std::getline(file, line))
	{
		StringParser::split(line, " ", words);

		if(!words[0].compare("v"))
			data.positions.emplace_back(std::stof(words[1]), std::stof(words[2]), std::stof(words[3]));
		else if(!words[0].compare("vt"))
			data.textureCoordinates.emplace_back(std::stof(words[1]), std::stof(words[2]));
		else if(!words[0].compare("vn"))
			data.normals.emplace_back(std::stof(words[1]), std::stof(words[2]), std::stof(words[3]));
		else if(!words[0].compare("f"))
			readFaceData(words, data);
		else if(!words[0].compare("usemtl"))
			submeshInfos.emplace_back(static_cast<uint32_t>(data.indices.size()), materialIDs[words[1]]);
	}
	file.close();

	LOG_VERBOSE("Mesh \"%s\" loaded.", fileName);
}

void mtd::ObjMeshLoader::loadRayTracingMesh
(
	const char* fileName,
	std::vector<Vertex>& vertices,
	std::vector<uint32_t>& indices,
	std::vector<uint16_t>& materialIndices,
	std::vector<Material>& meshMaterials,
	const MaterialInfo& materialInfo
)
{
	std::string objMeshPath{MTD_RESOURCES_PATH};
	objMeshPath.append("meshes/");
	objMeshPath.append(fileName);

	std::unordered_map<std::string, uint32_t> materialIDs;
	loadMaterials(objMeshPath, meshMaterials, materialIDs, materialInfo);

	std::string line;
	std::vector<std::string> words;
	uint16_t currentMaterialID = 0;

	ObjData data{vertices, indices};

	std::ifstream file;
	file.open(objMeshPath);
	while(std::getline(file, line))
	{
		StringParser::split(line, " ", words);

		if(!words[0].compare("v"))
			data.positions.emplace_back(std::stof(words[1]), std::stof(words[2]), std::stof(words[3]));
		else if(!words[0].compare("vt"))
			data.textureCoordinates.emplace_back(std::stof(words[1]), std::stof(words[2]));
		else if(!words[0].compare("vn"))
			data.normals.emplace_back(std::stof(words[1]), std::stof(words[2]), std::stof(words[3]));
		else if(!words[0].compare("f"))
		{
			const size_t triangleCount = readFaceData(words, data);
			materialIndices.insert(materialIndices.cend(), triangleCount, currentMaterialID);
		}
		else if(!words[0].compare("usemtl"))
			currentMaterialID = materialIDs[words[1]];
	}
	file.close();

	LOG_VERBOSE("Mesh \"%s\" loaded.", fileName);
}

void mtd::loadMaterial(std::string filePath, Material& material)
{
	filePath.replace(filePath.size() - 3, 3, "mtl");

	std::string line;
	std::vector<std::string> words;

	bool readFirstMaterial = false;

	std::ifstream file;
	file.open(filePath);
	while(std::getline(file, line))
	{
		StringParser::split(line, " ", words);

		if(!words[0].compare("newmtl"))
		{
			if(readFirstMaterial) break;
			readFirstMaterial = true;
		}
		else if(!words[0].compare("Kd"))
		{
			Vec4 color{std::stof(words[1]), std::stof(words[2]), std::stof(words[3]), 1.0f};
			material.addFloatData(MaterialFloatDataType::DiffuseColor, reinterpret_cast<float*>(&color));
		}
		else if(!words[0].compare("map_Kd"))
		{
			std::string diffuseTexturePath{MTD_RESOURCES_PATH};
			diffuseTexturePath.append("textures/");
			diffuseTexturePath.append(words[1]);
			material.addTexturePath(MaterialTextureType::DiffuseMap, std::move(diffuseTexturePath));
		}
		else if(!words[0].compare("Ke"))
		{
			Vec3 emission{std::stof(words[1]), std::stof(words[2]), std::stof(words[3])};
			material.addFloatData(MaterialFloatDataType::Emission, reinterpret_cast<float*>(&emission));
		}
		else if(!words[0].compare("Ni"))
		{
			float indexOfRefraction = std::stof(words[1]);
			material.addFloatData(MaterialFloatDataType::IndexOfRefraction, &indexOfRefraction);
		}
		else if(!words[0].compare("Pr"))
		{
			float roughness = std::stof(words[1]);
			material.addFloatData(MaterialFloatDataType::Roughness, &roughness);
		}
		else if(!words[0].compare("Pm"))
		{
			float metallic = std::stof(words[1]);
			material.addFloatData(MaterialFloatDataType::Metallic, &metallic);
		}
	}
	file.close();
}

void mtd::loadMaterials
(
	std::string filePath,
	std::vector<Material>& materials,
	std::unordered_map<std::string, uint32_t>& materialIDs,
	const MaterialInfo& materialInfo
)
{
	filePath.replace(filePath.size() - 3, 3, "mtl");

	std::string line;
	std::vector<std::string> words;

	uint32_t currentMaterialID = 0;
	uint32_t nextMaterialID = 0;

	std::ifstream file;
	file.open(filePath);
	while(std::getline(file, line))
	{
		StringParser::split(line, " ", words);

		if(!words[0].compare("newmtl"))
		{
			currentMaterialID = nextMaterialID;
			materialIDs[words[1]] = currentMaterialID;
			materials.emplace_back(materialInfo);
			nextMaterialID++;
		}
		else if(!words[0].compare("Kd"))
		{
			Vec4 color{std::stof(words[1]), std::stof(words[2]), std::stof(words[3]), 1.0f};
			materials[currentMaterialID]
				.addFloatData(MaterialFloatDataType::DiffuseColor, reinterpret_cast<float*>(&color));
		}
		else if(!words[0].compare("map_Kd"))
		{
			std::string diffuseTexturePath{MTD_RESOURCES_PATH};
			diffuseTexturePath.append("textures/");
			diffuseTexturePath.append(words[1]);
			materials[currentMaterialID]
				.addTexturePath(MaterialTextureType::DiffuseMap, std::move(diffuseTexturePath));
		}
		else if(!words[0].compare("Ke"))
		{
			Vec3 emission{std::stof(words[1]), std::stof(words[2]), std::stof(words[3])};
			materials[currentMaterialID]
				.addFloatData(MaterialFloatDataType::Emission, reinterpret_cast<float*>(&emission));
		}
		else if(!words[0].compare("Ni"))
		{
			float indexOfRefraction = std::stof(words[1]);
			materials[currentMaterialID]
				.addFloatData(MaterialFloatDataType::IndexOfRefraction, &indexOfRefraction);
		}
		else if(!words[0].compare("Pr"))
		{
			float roughness = std::stof(words[1]);
			materials[currentMaterialID].addFloatData(MaterialFloatDataType::Roughness, &roughness);
		}
		else if(!words[0].compare("Pm"))
		{
			float metallic = std::stof(words[1]);
			materials[currentMaterialID].addFloatData(MaterialFloatDataType::Metallic, &metallic);
		}
	}
	file.close();
}

size_t mtd::readFaceData(const std::vector<std::string>& words, ObjData& data)
{
	size_t triangleCount = words.size() - 3;
	for(size_t i = 0; i < triangleCount; i++)
	{
		readVertex(words[1], data);
		readVertex(words[2 + i], data);
		readVertex(words[3 + i], data);
	}
	return triangleCount;
}

void mtd::readVertex(const std::string& vertexDescription, ObjData& data)
{
	if(data.history.contains(vertexDescription))
	{
		data.indices.push_back(data.history[vertexDescription]);
		return;
	}

	uint32_t index = static_cast<uint32_t>(data.history.size());
	data.history.insert({vertexDescription, index});
	data.indices.push_back(index);

	std::vector<std::string> fullVertexData;
	StringParser::split(vertexDescription, "/", fullVertexData);

	Vec3 pos = data.positions[stol(fullVertexData[0]) - 1];

	Vec2 texCoord{0.0f, 0.0f};
	if(fullVertexData.size() > 1 && fullVertexData[1].size() > 0)
		texCoord = data.textureCoordinates[stol(fullVertexData[1]) - 1];

	Vec3 normal{0.0f, 0.0f, -1.0f};
	if(fullVertexData.size() > 2 && fullVertexData[2].size() > 0)
		normal = data.normals[stol(fullVertexData[2]) - 1];

	data.vertices.emplace_back(pos, texCoord, normal);
}
