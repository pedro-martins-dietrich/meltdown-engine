#include <pch.hpp>
#include "MeshPool.hpp"

#include "AssetLoad/MeshLoader.hpp"
#include "../Utils/Logger.hpp"

mtd::MeshPool::MeshPool(const Device& mtdDevice)
    : vertexBuffer
    {
        mtdDevice,
        vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eVertexBuffer,
        vk::MemoryPropertyFlagBits::eDeviceLocal
    },
    indexBuffer
    {
        mtdDevice,
        vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eIndexBuffer,
        vk::MemoryPropertyFlagBits::eDeviceLocal
    },
    submeshBuffer
    {
        mtdDevice,
        vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eIndexBuffer,
        vk::MemoryPropertyFlagBits::eDeviceLocal
    },
    commandHandler{mtdDevice}
{}

void mtd::MeshPool::loadMeshes(const std::vector<std::string>& meshFiles)
{
    if(meshFiles.size() == 0)
    {
        anyMeshLoaded = false;
        return;
    }

    std::vector<std::byte> vertexData;
    std::vector<uint32_t> indexData;
    meshes.clear();

    for(const std::string_view file: meshFiles)
        MeshLoader::loadMesh(file, vertexData, indexData, meshes);

    loadToGpu(vertexData, indexData);
    anyMeshLoaded = true;

    LOG_INFO("Loaded %d meshes.", meshFiles.size());
}

void mtd::MeshPool::createMeshDescriptors
(
    DescriptorSetHandler& descriptorSetHandler,
    uint32_t vertexBinding, uint32_t indexBinding, uint32_t submeshBinding
) const
{
    if(!anyMeshLoaded)
    {
        LOG_VERBOSE("No meshes loaded. Mesh descriptors will not be used.");
        return;
    }

    descriptorSetHandler.assignExternalResourcesToDescriptor(0U, vertexBinding, vertexBuffer);
    descriptorSetHandler.assignExternalResourcesToDescriptor(0U, indexBinding, indexBuffer);
    descriptorSetHandler.assignExternalResourcesToDescriptor(0U, submeshBinding, submeshBuffer);
}

void mtd::MeshPool::bindBuffers(const vk::CommandBuffer& commandBuffer) const
{
    vk::DeviceSize offset{0};
    commandBuffer.bindVertexBuffers(0U, 1U, &(vertexBuffer.getBuffer()), &offset);
    commandBuffer.bindIndexBuffer(indexBuffer.getBuffer(), offset, vk::IndexType::eUint32);
}

void mtd::MeshPool::loadToGpu(const std::vector<std::byte>& vertexData, const std::vector<uint32_t>& indexData)
{
    assert(!meshes.empty() && "Cannot load mesh data to the GPU if there are no meshes.");

    std::vector<SubmeshData> submeshes;
    submeshes.reserve(meshes.back().submeshOffset + meshes.back().submeshes.size());
    for(const MeshData& mesh: meshes)
        submeshes.insert(submeshes.end(), mesh.submeshes.begin(), mesh.submeshes.end());

    if(!gpuBuffersCreated)
    {
        vertexBuffer.createDeviceLocal(commandHandler, vertexData.size(), vertexData.data());
        indexBuffer.createDeviceLocal(commandHandler, sizeof(uint32_t) * indexData.size(), indexData.data());
        submeshBuffer.createDeviceLocal(commandHandler, sizeof(SubmeshData) * submeshes.size(), submeshes.data());

        gpuBuffersCreated = true;
        return;
    }

    vertexBuffer.resizeBuffer(commandHandler, vertexData.size());
    vertexBuffer.copyMemoryToBuffer(vertexData.size(), vertexData.data());
    indexBuffer.resizeBuffer(commandHandler, sizeof(uint32_t) * indexData.size());
    indexBuffer.copyMemoryToBuffer(sizeof(uint32_t) * indexData.size(), indexData.data());
    submeshBuffer.resizeBuffer(commandHandler, sizeof(SubmeshData) * submeshes.size());
    submeshBuffer.copyMemoryToBuffer(sizeof(SubmeshData) * submeshes.size(), submeshes.data());
}
