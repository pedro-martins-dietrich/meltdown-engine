#include <pch.hpp>
#include "MeshImporter.hpp"

#include "../../Utils/Logger.hpp"
#include "../../Utils/StringParser.hpp"

namespace mtd
{
    constexpr uint64_t MESH_MAGIC = "MTD_MESH"_u64;
    constexpr uint32_t MESH_FILE_VERSION = 1U;
}

bool mtd::MeshImporter::loadMesh
(
    const std::string_view filePath,
    std::vector<std::byte>& vertexData,
    std::vector<uint32_t>& indexData,
    std::vector<SubmeshData>& submeshes
)
{
    std::ifstream meshFile{filePath.data(), std::ios::binary | std::ios::ate};
    if(!meshFile)
    {
        LOG_WARNING("Failed to find mesh file \"%s\" for loading.", filePath.data());
        return false;
    }

    std::streamsize meshFileSize = meshFile.tellg();
    if(meshFileSize < static_cast<std::streamsize>(sizeof(AssetHeader) + sizeof(AssetBlockHeader)))
    {
        LOG_WARNING("Invalid header for mesh file \"%s\".", filePath.data());
        return false;
    }

    AssetHeader meshHeader;
    meshFile.seekg(0, std::ios::beg);
    meshFile.read(reinterpret_cast<char*>(&meshHeader), sizeof(AssetHeader));

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

    std::streamoff currentOffset = meshFile.tellg();

    while(currentOffset < meshFileSize)
    {
        if(meshFileSize - currentOffset < blockHeader.blockSize)
        {
            LOG_WARNING("Invalid block [%d] size in mesh file \"%s\". Skipping...", blockHeader.blockID, filePath.data());
            break;
        }

        size_t oldBufferSize = 0U;
        switch(blockHeader.blockID)
        {
            case "Vertices"_u64:
                oldBufferSize = vertexData.size();
                vertexData.resize(oldBufferSize + blockHeader.blockSize);
                meshFile.read(reinterpret_cast<char*>(vertexData.data() + oldBufferSize), blockHeader.blockSize);
                break;

            case "Indices\0"_u64:
                oldBufferSize = indexData.size();
                indexData.resize(oldBufferSize + (blockHeader.blockSize / sizeof(uint32_t)));
                meshFile.read(reinterpret_cast<char*>(indexData.data() + oldBufferSize), blockHeader.blockSize);
                break;

            case "Submesh\0"_u64:
                oldBufferSize = submeshes.size();
                submeshes.resize(oldBufferSize + (blockHeader.blockSize / sizeof(SubmeshData)));
                meshFile.read(reinterpret_cast<char*>(submeshes.data() + oldBufferSize), blockHeader.blockSize);
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

    LOG_VERBOSE("Mesh loaded from \"%s\".", filePath.data());
    return true;
}
