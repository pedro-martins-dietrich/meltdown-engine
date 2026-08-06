#include <pch.hpp>
#include "MeshPool.hpp"

#include "AssetLoad/MeshLoader.hpp"
#include "../Utils/Logger.hpp"

void mtd::MeshPool::loadMeshes(ResourceManager& resourceManager, const std::vector<std::string>& meshFiles)
{
    std::vector<std::byte> vertexData;
    std::vector<uint32_t> indexData;
    meshes.clear();

    for(const std::string_view file: meshFiles)
        MeshLoader::loadMesh(file, vertexData, indexData, meshes);

    loadToGpu(resourceManager, vertexData, indexData);

    LOG_INFO("Loaded %d meshes.", meshFiles.size());
}

void mtd::MeshPool::loadToGpu
(
    ResourceManager& resourceManager,
    const std::vector<std::byte>& vertexData,
    const std::vector<uint32_t>& indexData
)
{
    assert(!meshes.empty() && "Cannot load mesh data to the GPU if there are no meshes.");

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
}
