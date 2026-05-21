#include <pch.hpp>
#include "MeshPool.hpp"

#include "Import/MeshImporter.hpp"
#include "../Utils/Logger.hpp"

mtd::MeshPool::MeshPool(const Device& mtdDevice)
    : vertexBuffer{mtdDevice, vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal},
    indexBuffer{mtdDevice, vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal},
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
    submeshes.clear();

    for(const std::string_view file: meshFiles)
    {
        MeshImporter::loadMesh(file, vertexData, indexData, submeshes);
    }

    loadToGpu(vertexData, indexData);
    anyMeshLoaded = true;

    LOG_INFO("Loaded %d meshes.", meshFiles.size());
}

void mtd::MeshPool::createMeshDescriptors
(
    DescriptorSetHandler& descriptorSetHandler,
    uint32_t vertexBinding, uint32_t indexBinding
) const
{
    if(!anyMeshLoaded)
    {
        LOG_VERBOSE("No meshes loaded. Mesh descriptors will not be used.");
        return;
    }

    descriptorSetHandler.assignExternalResourcesToDescriptor(0U, vertexBinding, vertexBuffer);
    descriptorSetHandler.assignExternalResourcesToDescriptor(0U, indexBinding, indexBuffer);
}

void mtd::MeshPool::loadToGpu(const std::vector<std::byte>& vertexData, const std::vector<uint32_t>& indexData)
{
    if(!gpuBuffersCreated)
    {
        vertexBuffer.createDeviceLocal(commandHandler, vertexData.size(), vertexData.data());
        indexBuffer.createDeviceLocal(commandHandler, sizeof(uint32_t) * indexData.size(), indexData.data());

        gpuBuffersCreated = true;
        return;
    }

    vertexBuffer.resizeBuffer(commandHandler, vertexData.size());
    vertexBuffer.copyMemoryToBuffer(vertexData.size(), vertexData.data());
    indexBuffer.resizeBuffer(commandHandler, sizeof(uint32_t) * indexData.size());
    indexBuffer.copyMemoryToBuffer(sizeof(uint32_t) * indexData.size(), indexData.data());
}
