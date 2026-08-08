#include <pch.hpp>
#include "MeshLoader.hpp"

#include "../Utils/Logger.hpp"
#include "../Utils/StringParser.hpp"

namespace mtd::MeshLoader
{
    constexpr uint64_t MESH_MAGIC = "MTD_MESH"_u64;
    constexpr uint64_t MESH_FILE_VERSION = 1UL;

    // Mesh asset file header
    struct MeshHeader : AssetHeader
    {
        uint32_t vertexStride;
        Vec3 centerAABB = Vec3{0.0f};
        Vec3 extentAABB = Vec3{0.0f};
    };

    // Loads a .mesh file and appends it to the mesh data
    static bool loadFromFile
    (
        const std::string_view filePath,
        std::vector<std::byte>& vertexData,
        std::vector<uint32_t>& indexData,
        std::vector<MeshData>& meshes
    );
}

void mtd::MeshLoader::loadMeshes
(
    ResourceManager& resourceManager,
    const std::vector<std::string>& meshFiles,
    std::vector<MeshData>& meshes,
    ResourceID& vertexBufferID,
    ResourceID& indexBufferID,
    ResourceID& submeshBufferID
)
{
    std::vector<std::byte> vertexData;
    std::vector<uint32_t> indexData;
    meshes.clear();

    for(const std::string_view file: meshFiles)
        MeshLoader::loadFromFile(file, vertexData, indexData, meshes);
    if(meshes.empty()) return;

    std::vector<SubmeshData> submeshes;
    submeshes.reserve(meshes.back().submeshOffset + meshes.back().submeshes.size());
    for(const MeshData& mesh: meshes)
        submeshes.insert(submeshes.end(), mesh.submeshes.begin(), mesh.submeshes.end());

    vertexBufferID = resourceManager.createBuffer
    (
        "VertexBuffer", GpuBufferType::Vertex, GpuMemoryUsage::GpuOnly,
        vertexData.size(), vertexData.data()
    );
    indexBufferID = resourceManager.createBuffer
    (
        "IndexBuffer", GpuBufferType::Index, GpuMemoryUsage::GpuOnly,
        sizeof(uint32_t) * indexData.size(), indexData.data()
    );
    submeshBufferID = resourceManager.createBuffer
    (
        "SubmeshBuffer", GpuBufferType::Index, GpuMemoryUsage::GpuOnly,
        sizeof(SubmeshData) * submeshes.size(), submeshes.data()
    );

    LOG_INFO("Loaded %d meshes.", meshFiles.size());
}

bool mtd::MeshLoader::loadFromFile
(
    const std::string_view filePath,
    std::vector<std::byte>& vertexData,
    std::vector<uint32_t>& indexData,
    std::vector<MeshData>& meshes
)
{
    std::ifstream meshFile{filePath.data(), std::ios::binary | std::ios::ate};
    if(!meshFile)
    {
        LOG_WARNING("Failed to find mesh file \"%s\" for loading.", filePath.data());
        return false;
    }

    std::streamsize meshFileSize = meshFile.tellg();
    if(meshFileSize < static_cast<std::streamsize>(sizeof(MeshHeader) + sizeof(AssetBlockHeader)))
    {
        LOG_WARNING("Invalid header for mesh file \"%s\".", filePath.data());
        return false;
    }

    MeshHeader meshHeader;
    meshFile.seekg(0, std::ios::beg);
    meshFile.read(reinterpret_cast<char*>(&meshHeader), sizeof(MeshHeader));

    bool validMeshFile = true;
    validMeshFile &= (meshHeader.magic == MESH_MAGIC);
    validMeshFile &= (meshHeader.version == MESH_FILE_VERSION);
    if(!validMeshFile)
    {
        LOG_WARNING("Invalid mesh file \"%s\".", filePath.data());
        return false;
    }

    AssetBlockHeader blockHeader;
    meshFile.read(reinterpret_cast<char*>(&blockHeader), sizeof(AssetBlockHeader));

    MeshData meshData{};
    meshData.vertexStride = meshHeader.vertexStride;
    meshData.indexOffset = static_cast<uint32_t>(indexData.size());
    meshData.materialSlotCount = 0U;
    meshData.centerAABB = meshHeader.centerAABB;
    meshData.extentAABB = meshHeader.extentAABB;
    meshData.submeshOffset = 0U;
    if(!meshes.empty())
        meshData.submeshOffset = meshes.back().submeshOffset + meshes.back().submeshes.size();

    std::streamoff currentOffset = meshFile.tellg();
    while(currentOffset < meshFileSize)
    {
        if(meshFileSize - currentOffset < blockHeader.blockSize)
        {
            LOG_WARNING
            (
                "Invalid block [%d] size in mesh file \"%s\". Skipping the rest of the file...",
                blockHeader.blockID, filePath.data()
            );
            break;
        }

        size_t oldBufferSize = 0;
        size_t vertexOffset = 0;
        switch(blockHeader.blockID)
        {
            case "Vertices"_u64:
                oldBufferSize = vertexData.size();
                if(oldBufferSize % meshData.vertexStride == 0)
                    vertexOffset = oldBufferSize;
                else
                    vertexOffset = oldBufferSize + meshData.vertexStride - (oldBufferSize % meshData.vertexStride);
                meshData.vertexOffset = static_cast<uint32_t>(vertexOffset / meshData.vertexStride);
                vertexData.resize(oldBufferSize + blockHeader.blockSize);
                meshFile.read(reinterpret_cast<char*>(vertexData.data() + oldBufferSize), blockHeader.blockSize);
                break;

            case "Indices\0"_u64:
                indexData.resize(meshData.indexOffset + (blockHeader.blockSize / sizeof(uint32_t)));
                meshFile.read(reinterpret_cast<char*>(indexData.data() + meshData.indexOffset), blockHeader.blockSize);
                break;

            case "Submesh\0"_u64:
                meshData.submeshes.resize(blockHeader.blockSize / sizeof(SubmeshData));
                meshFile.read(reinterpret_cast<char*>(meshData.submeshes.data()), blockHeader.blockSize);
                for(const SubmeshData& submesh: meshData.submeshes)
                {
                    if(submesh.materialSlot + 1U > meshData.materialSlotCount)
                        meshData.materialSlotCount = submesh.materialSlot + 1U;
                }
                break;

            default:
                LOG_WARNING("Unknown block [%d] in mesh file \"%s\". Skipping...", blockHeader.blockID, filePath.data());
                meshFile.seekg(blockHeader.blockSize, std::ios_base::cur);
        }

        currentOffset = meshFile.tellg();
        if(meshFileSize < currentOffset + static_cast<std::streamsize>(sizeof(AssetBlockHeader))) break;

        meshFile.read(reinterpret_cast<char*>(&blockHeader), sizeof(AssetBlockHeader));
        currentOffset = meshFile.tellg();
    }

    meshFile.close();

    meshes.emplace_back(std::move(meshData));

    LOG_VERBOSE("Mesh loaded from \"%s\".", filePath.data());
    return true;
}
