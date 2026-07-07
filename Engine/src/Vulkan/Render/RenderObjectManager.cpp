#include <pch.hpp>
#include "RenderObjectManager.hpp"

mtd::RenderObjectManager::RenderObjectManager(const Device& mtdDevice)
    : commandHandler{mtdDevice},
    renderObjectBuffer
    {
        mtdDevice,
        sizeof(RenderObject),
        vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eStorageBuffer
            | vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
    }
{}

void mtd::RenderObjectManager::createFrameRenderObjects
(
    const MeshPool& meshPool,
    const std::vector<SceneInstance>& sceneInstances,
    std::vector<DrawBatch>& drawBatches,
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
    const MeshData& firstMesh = meshPool.getMesh(firstInstance->meshID);
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

    for(size_t i = 1; i < visibleInstances.size(); i++)
    {
        const SceneInstance* pInstance = visibleInstances[i];
        const MeshData& mesh = meshPool.getMesh(firstInstance->meshID);
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

    updateBufferData(descriptorSetHandler);

    renderObjectCount = renderObjects.size();
    renderObjects.clear();
}

void mtd::RenderObjectManager::bindBuffer(vk::CommandBuffer commandBuffer) const
{
    vk::DeviceSize offset{0UL};
    commandBuffer.bindVertexBuffers(1U, 1U, &(renderObjectBuffer.getBuffer()), &offset);
}

void mtd::RenderObjectManager::createDescriptor(DescriptorSetHandler& descriptorSetHandler, uint32_t binding) const
{
    bufferDescriptorBinding = binding;
    descriptorSetHandler.assignExternalResourcesToDescriptor(0U, binding, renderObjectBuffer);
}

void mtd::RenderObjectManager::updateBufferData(DescriptorSetHandler& descriptorSetHandler)
{
    vk::DeviceSize minimumBufferSize = renderObjects.size() * sizeof(RenderObject);
	if(minimumBufferSize > renderObjectBuffer.getSize())
    {
		renderObjectBuffer.resizeBuffer(commandHandler, minimumBufferSize);
        descriptorSetHandler.assignExternalResourcesToDescriptor(0U, bufferDescriptorBinding, renderObjectBuffer);
        descriptorSetHandler.writeDescriptor(0U, bufferDescriptorBinding);
    }

    renderObjectBuffer.copyMemoryToBuffer(renderObjects.size() * sizeof(RenderObject), renderObjects.data());
}
