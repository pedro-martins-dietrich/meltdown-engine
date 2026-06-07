#pragma once

#include "../../Scene/InstanceManager.hpp"
#include "../Device/GpuBuffer.hpp"

namespace mtd
{
    // Handles the objects that should be rendered at each frame
    class RenderObjectManager
    {
        public:
            RenderObjectManager(const Device& mtdDevice);
            ~RenderObjectManager() = default;

            RenderObjectManager(const RenderObjectManager&) = delete;
            RenderObjectManager& operator=(const RenderObjectManager&) = delete;

            // Creates the render objects and the draw batches from the scene instances
            void createFrameRenderObjects
            (
                const std::vector<SceneInstance>& sceneInstances, std::vector<DrawBatch>& drawBatches
            );
            // Binds the render objects buffer
            void bindRenderObjectsBuffer(vk::CommandBuffer commandBuffer) const;

        private:
            // GPU buffer for the render objects that will be used in the current frame
            GpuBuffer renderObjectsBuffer;
            // Render object manager command handler
            CommandHandler commandHandler;

            // List of instances visible in the current frame
            std::vector<const SceneInstance*> visibleInstances;
            // List of render objects for the current frame
            std::vector<RenderObject> renderObjects;

            // Updates the render objects buffer contents
            void updateRenderObjectsBuffer();
    };
}
