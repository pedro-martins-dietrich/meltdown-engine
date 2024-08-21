#include "ObjMeshLoader.hpp"

#include <fstream>
#include <unordered_map>

#include <iostream>

#include "../../Utils/FileHandler.hpp"
#include "../../Utils/Logger.hpp"
#include "../../Utils/StringParser.hpp"

// Data bundle for the .obj loader
struct ObjData
{
	std::vector<mtd::Vertex>& vertices;
	std::vector<uint32_t>& indices;
	std::vector<glm::vec3> positions;
	std::vector<glm::vec2> textureCoordinates;
	std::vector<glm::vec3> normals;
	std::unordered_map<std::string, uint32_t> history;
	std::unordered_map<std::string, glm::vec3> colors;
	glm::vec3 brushColor;
};

// Parses each triangle of a face
static void readFaceData(const std::vector<std::string>& words, ObjData& data);
// Parses a vertex
static void readVertex(const std::string& vertexDescription, ObjData& data);

// Loads a mesh from an Wavefront .obj file
void mtd::ObjMeshLoader::load
(
	const char* fileName,
	std::vector<Vertex>& vertices,
	std::vector<uint32_t>& indices,
	std::string& diffuseTexturePath
)
{
	std::string objMeshPath{MTD_RESOURCES_PATH};
	objMeshPath.append("meshes/");
	objMeshPath.append(fileName);

	std::string mtlMeshPath{objMeshPath};
	mtlMeshPath.replace(mtlMeshPath.size() - 3, 3, "mtl");

	std::string line;
	std::vector<std::string> words;
	std::string materialName;

	std::ifstream file;

	ObjData data{vertices, indices};
	data.brushColor = glm::vec3{1.0f};

	// Material file (.mtl)
	file.open(mtlMeshPath);
	while(std::getline(file, line))
	{
		StringParser::split(line, " ", words);

		if(!words[0].compare("newmtl"))
		{
			materialName = words[1];
			continue;
		}
		if(!words[0].compare("Kd"))
		{
			data.brushColor =
				glm::vec3{std::stof(words[1]), std::stof(words[2]), std::stof(words[3])};
			data.colors.insert({materialName, data.brushColor});
			continue;
		}
		if(!words[0].compare("map_Kd"))
		{
			diffuseTexturePath = "meshes/";
			diffuseTexturePath.append(fileName);
			diffuseTexturePath = diffuseTexturePath.substr(0, diffuseTexturePath.find_last_of("/\\") + 1);
			diffuseTexturePath.append(words[1]);
		}
	}
	file.close();

	// Wavefront file (.obj)
	file.open(objMeshPath);
	while(std::getline(file, line))
	{
		StringParser::split(line, " ", words);

		if(!words[0].compare("v"))
		{
			data.positions.emplace_back
			(
				std::stof(words[1]), std::stof(words[2]), std::stof(words[3])
			);
			continue;
		}
		if(!words[0].compare("vt"))
		{
			data.textureCoordinates.emplace_back(std::stof(words[1]), std::stof(words[2]));
			continue;
		}
		if(!words[0].compare("vn"))
		{
			data.normals.emplace_back
			(
				std::stof(words[1]), std::stof(words[2]), std::stof(words[3])
			);
			continue;
		}
		if(!words[0].compare("f"))
		{
			readFaceData(words, data);
			continue;
		}
		if(!words[0].compare("usemtl"))
		{
			if(data.colors.contains(words[1]))
				data.brushColor = data.colors[words[1]];
			else
				data.brushColor = glm::vec3{1.0f};
		}
	}
	file.close();

	LOG_VERBOSE("Mesh \"%s\" loaded.", fileName);
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

	glm::vec3 pos = data.positions[stol(fullVertexData[0]) - 1];

	glm::vec2 texCoord{0.0f, 0.0f};
	if(fullVertexData.size() > 1 && fullVertexData[1].size() > 0)
		texCoord = data.textureCoordinates[stol(fullVertexData[1]) - 1];

	glm::vec3 normal{0.0f, 0.0f, -1.0f};
	if(fullVertexData.size() > 2 && fullVertexData[2].size() > 0)
		normal = data.normals[stol(fullVertexData[2]) - 1];

	data.vertices.emplace_back(pos, texCoord, normal, data.brushColor);
}
