#pragma once

#include "../../Scene/InstanceManager.hpp"
#include "../../AssetManager/MeshPool.hpp"

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

            // Getter
            uint32_t getRenderObjectCount() const { return renderObjectCount; }

            // Creates the render objects and the draw batches from the scene instances
            void createFrameRenderObjects
            (
                const MeshPool& meshPool,
                const std::vector<SceneInstance>& sceneInstances,
                std::vector<DrawBatch>& drawBatches,
                DescriptorSetHandler& descriptorSetHandler
            );

            // Binds the render object buffer
            void bindBuffer(vk::CommandBuffer commandBuffer) const;
            // Creates the render object descriptor for the render object buffer
            void createDescriptor(DescriptorSetHandler& descriptorSetHandler, uint32_t binding) const;

        private:
            // GPU buffer for the render objects that will be used in the current frame
            GpuBuffer renderObjectBuffer;
            // Render object manager command handler
            CommandHandler commandHandler;

            // Descriptor binding used for the render object GPU buffer
            mutable uint32_t bufferDescriptorBinding;

            // List of instances visible in the current frame
            std::vector<const SceneInstance*> visibleInstances;
            // List of render objects for the current frame
            std::vector<RenderObject> renderObjects;
            // Count of active render objects in the GPU buffer
            uint32_t renderObjectCount = 0U;

            // Updates the render objects buffer contents
            void updateBufferData(DescriptorSetHandler& descriptorSetHandler);
    };
}
