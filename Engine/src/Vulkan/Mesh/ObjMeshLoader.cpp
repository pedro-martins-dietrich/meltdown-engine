#include <pch.hpp>
#include "ObjMeshLoader.hpp"

#include "../../Utils/Logger.hpp"
#include "../../Utils/FileHandler.hpp"
#include "../../Utils/StringParser.hpp"

// Data bundle for the .obj loader
struct ObjData
{
	std::vector<mtd::Vertex>& vertices;
	std::vector<uint32_t>& indices;
	std::vector<mtd::Vec3> positions;
	std::vector<glm::vec2> textureCoordinates;
	std::vector<mtd::Vec3> normals;
	std::unordered_map<std::string, uint32_t> history;
};

// Loads a single material from a .mtl file
static void loadMaterial(std::string filePath, mtd::Material& material);
// Loads the materials from a .mtl file
static void loadMaterials
(
	std::string filePath,
	std::vector<mtd::Material>& materials,
	std::unordered_map<std::string, uint32_t>& materialIDs,
	const mtd::MaterialInfo& materialInfo
);

// Parses each triangle of a face
static void readFaceData(const std::vector<std::string>& words, ObjData& data);
// Parses a vertex
static void readVertex(const std::string& vertexDescription, ObjData& data);

// Loads a default 3D mesh from an Wavefront file
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

// Loads a mesh with multiple materials from an Wavefront file
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

// Loads a single material from a .mtl file
void loadMaterial(std::string filePath, mtd::Material& material)
{
	filePath.replace(filePath.size() - 3, 3, "mtl");

	std::string line;
	std::vector<std::string> words;

	bool readFirstMaterial = false;

	std::ifstream file;
	file.open(filePath);
	while(std::getline(file, line))
	{
		mtd::StringParser::split(line, " ", words);

		if(!words[0].compare("newmtl"))
		{
			if(readFirstMaterial) break;
			readFirstMaterial = true;
		}
		else if(!words[0].compare("Kd"))
		{
			mtd::Vec4 color{std::stof(words[1]), std::stof(words[2]), std::stof(words[3]), 1.0f};
			material.addFloatData(mtd::MaterialFloatDataType::DiffuseColor, reinterpret_cast<float*>(&color));
		}
		else if(!words[0].compare("map_Kd"))
		{
			std::string diffuseTexturePath{filePath};
			diffuseTexturePath = diffuseTexturePath.substr(0, diffuseTexturePath.find_last_of("/\\") + 1);
			diffuseTexturePath.append(words[1]);
			material.addTexturePath(mtd::MaterialTextureType::DiffuseMap, std::move(diffuseTexturePath));
		}
	}
	file.close();
}

// Loads the materials from a .mtl file
void loadMaterials
(
	std::string filePath,
	std::vector<mtd::Material>& materials,
	std::unordered_map<std::string, uint32_t>& materialIDs,
	const mtd::MaterialInfo& materialInfo
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
		mtd::StringParser::split(line, " ", words);

		if(!words[0].compare("newmtl"))
		{
			currentMaterialID = nextMaterialID;
			materialIDs[words[1]] = currentMaterialID;
			materials.emplace_back(materialInfo);
			nextMaterialID++;
		}
		else if(!words[0].compare("Kd"))
		{
			mtd::Vec4 color{std::stof(words[1]), std::stof(words[2]), std::stof(words[3]), 1.0f};
			materials[currentMaterialID]
				.addFloatData(mtd::MaterialFloatDataType::DiffuseColor, reinterpret_cast<float*>(&color));
		}
		else if(!words[0].compare("map_Kd"))
		{
			std::string diffuseTexturePath{filePath};
			diffuseTexturePath = diffuseTexturePath.substr(0, diffuseTexturePath.find_last_of("/\\") + 1);
			diffuseTexturePath.append(words[1]);
			materials[currentMaterialID]
				.addTexturePath(mtd::MaterialTextureType::DiffuseMap, std::move(diffuseTexturePath));
		}
	}
	file.close();
}

// Parses each triangle of a face
void readFaceData(const std::vector<std::string>& words, ObjData& data)
{
	size_t triangleCount = words.size() - 3;
	for(size_t i = 0; i < triangleCount; i++)
	{
		readVertex(words[1], data);
		readVertex(words[2 + i], data);
		readVertex(words[3 + i], data);
	}
}

// Parses a vertex
void readVertex(const std::string& vertexDescription, ObjData& data)
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
	mtd::StringParser::split(vertexDescription, "/", fullVertexData);

	mtd::Vec3 pos = data.positions[stol(fullVertexData[0]) - 1];

	glm::vec2 texCoord{0.0f, 0.0f};
	if(fullVertexData.size() > 1 && fullVertexData[1].size() > 0)
		texCoord = data.textureCoordinates[stol(fullVertexData[1]) - 1];

	mtd::Vec3 normal{0.0f, 0.0f, -1.0f};
	if(fullVertexData.size() > 2 && fullVertexData[2].size() > 0)
		normal = data.normals[stol(fullVertexData[2]) - 1];

	data.vertices.emplace_back(pos, texCoord, normal);
}
