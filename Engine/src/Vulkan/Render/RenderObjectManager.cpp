#include <pch.hpp>
#include "RenderObjectManager.hpp"

#include "../../Utils/Logger.hpp"

void mtd::RenderObjectManager::createBuffer(ResourceManager& resourceManager)
{
    renderObjectBufferID = resourceManager.createBuffer
    (
        "RenderObjectsBuffer",
        GpuBufferType::Vertex | GpuBufferType::TransferSource,
        GpuMemoryUsage::CpuUpload,
        sizeof(RenderObject)
    );
}

void mtd::RenderObjectManager::createFrameRenderObjects
(
    ResourceManager& resourceManager,
    const std::vector<MeshData>& meshes,
    const std::vector<SceneInstance>& sceneInstances,
    std::vector<DrawBatch>& drawBatches,
    DescriptorManager& descriptorManager,
    DescriptorSetHandler& descriptorSetHandler
)
{
    for(const SceneInstance& instance: sceneInstances)
    {
        if(instance.visible)
            visibleInstances.push_back(&instance);
    }

    if(visibleInstances.empty()) return;

    std::sort
    (
        visibleInstances.begin(),
        visibleInstances.end(),
        [](const SceneInstance* a, const SceneInstance* b)
        {
            if(a->pipelineID != b->pipelineID)
                return a->pipelineID < b->pipelineID;
            return a->meshID < b->meshID;
        }
    );

    const SceneInstance* firstInstance = visibleInstances.front();
    const MeshData& firstMesh = meshes[firstInstance->meshID];
    renderObjects.push_back(RenderObject
    {
        firstInstance->transform,
        firstInstance->materialSetID,
        firstMesh.submeshOffset,
        static_cast<uint32_t>(firstMesh.submeshes.size()),
        firstMesh.vertexOffset,
        firstMesh.centerAABB, 0.0f,
        firstMesh.extentAABB, 0.0f
    });

    drawBatches.push_back(DrawBatch{firstInstance->pipelineID, firstInstance->meshID, 0U, 0U});
    DrawBatch* pCurrentBatch = &(drawBatches.back());

    for(size_t i = 1UL; i < visibleInstances.size(); i++)
    {
        const SceneInstance* pInstance = visibleInstances[i];
        const MeshData& mesh = meshes[firstInstance->meshID];
        renderObjects.push_back(RenderObject{
            pInstance->transform,
            pInstance->materialSetID,
            mesh.submeshOffset,
            static_cast<uint32_t>(mesh.submeshes.size()),
            mesh.vertexOffset,
            mesh.centerAABB, 0.0f,
            mesh.extentAABB, 0.0f
        });

        if(pCurrentBatch->pipelineID != pInstance->pipelineID || pCurrentBatch->meshID != pInstance->meshID)
        {
            pCurrentBatch->instanceCount = i - pCurrentBatch->firstInstance;
            drawBatches.push_back(DrawBatch{pInstance->pipelineID, pInstance->meshID, static_cast<uint32_t>(i), 0U});
            pCurrentBatch = &(drawBatches.back());
        }
    }
    pCurrentBatch->instanceCount = visibleInstances.size() - pCurrentBatch->firstInstance;

    visibleInstances.clear();

    updateBufferData(resourceManager, descriptorManager, descriptorSetHandler);

    renderObjectCount = renderObjects.size();
    renderObjects.clear();
}

void mtd::RenderObjectManager::updateDescriptor
(
    const ResourceManager& resourceManager, DescriptorSetHandler& descriptorSetHandler
) const
{
    vk::DescriptorBufferInfo bufferInfo{};
    if(resourceManager.fetchDescriptorBufferInfo(renderObjectBufferID, bufferInfo))
        descriptorSetHandler.assignBuffer(8U, bufferInfo);
}

void mtd::RenderObjectManager::bindBuffer
(
    const ResourceManager& resourceManager, vk::CommandBuffer commandBuffer
) const
{
    assert(renderObjectBufferID != 0U && "The render objects buffer must be created before binding it.");

    vk::DeviceSize offset{0UL};
    vk::Buffer buffer = resourceManager.getVulkanBuffer(renderObjectBufferID);
    if(!buffer)
    {
        LOG_ERROR("Failed to bind render objects buffer.");
        return;
    }

    commandBuffer.bindVertexBuffers(1U, 1U, &buffer, &offset);
}

void mtd::RenderObjectManager::updateBufferData
(
    ResourceManager& resourceManager, DescriptorManager& descriptorManager, DescriptorSetHandler& descriptorSetHandler
)
{
    assert(renderObjectBufferID != 0U && "The render objects buffer must be created before updating it.");

    uint64_t minimumBufferSize = renderObjects.size() * sizeof(RenderObject);
    uint64_t currentBufferSize = resourceManager.getBufferSize(renderObjectBufferID);
    if(currentBufferSize < minimumBufferSize)
    {
        resourceManager.resizeBuffer(renderObjectBufferID, minimumBufferSize);
        updateDescriptor(resourceManager, descriptorSetHandler);
        descriptorManager.updateResourceDescriptors(renderObjectBufferID);
    }

    if(!resourceManager.updateBufferData(renderObjectBufferID, minimumBufferSize, renderObjects.data()))
        LOG_ERROR("Failed to update render objects buffer data.");
}
