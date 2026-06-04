#include <pch.hpp>
#include "RenderObjectManager.hpp"

mtd::RenderObjectManager::RenderObjectManager(const Device& mtdDevice)
    : commandHandler{mtdDevice},
    renderObjectsBuffer
    {
        mtdDevice,
        sizeof(RenderObject),
        vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
    }
{}

void mtd::RenderObjectManager::createFrameRenderObjects
(
    const std::vector<SceneInstance>& sceneInstances, std::vector<DrawBatch>& drawBatches
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
    renderObjects.push_back(RenderObject{firstInstance->transform, firstInstance->materialSetID});

    drawBatches.push_back(DrawBatch{firstInstance->pipelineID, firstInstance->meshID, 0U, 0U});
    DrawBatch* pCurrentBatch = &(drawBatches.back());

    for(size_t i = 1; i < visibleInstances.size(); i++)
    {
        const SceneInstance* pInstance = visibleInstances[i];
        renderObjects.push_back(RenderObject{pInstance->transform, pInstance->materialSetID});

        if(pCurrentBatch->pipelineID != pInstance->pipelineID || pCurrentBatch->meshID != pInstance->meshID)
        {
            pCurrentBatch->instanceCount = i - pCurrentBatch->firstInstance;
            drawBatches.push_back(DrawBatch{pInstance->pipelineID, pInstance->meshID, static_cast<uint32_t>(i), 0U});
            pCurrentBatch = &(drawBatches.back());
        }
    }
    pCurrentBatch->instanceCount = visibleInstances.size() - pCurrentBatch->firstInstance;

    visibleInstances.clear();

    updateRenderObjectsBuffer();
    renderObjects.clear();
}

void mtd::RenderObjectManager::bindRenderObjectsBuffer(vk::CommandBuffer commandBuffer) const
{
    vk::DeviceSize offset{0UL};
    commandBuffer.bindVertexBuffers(1U, 1U, &(renderObjectsBuffer.getBuffer()), &offset);
}

void mtd::RenderObjectManager::updateRenderObjectsBuffer()
{
    vk::DeviceSize minimumBufferSize = renderObjects.size() * sizeof(RenderObject);
	if(minimumBufferSize > renderObjectsBuffer.getSize())
		renderObjectsBuffer.resizeBuffer(commandHandler, minimumBufferSize);

    renderObjectsBuffer.copyMemoryToBuffer(renderObjects.size() * sizeof(RenderObject), renderObjects.data());
}
