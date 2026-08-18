#pragma once

#include "../Descriptors/DescriptorSetHandler.hpp"
#include "../Descriptors/DescriptorManager.hpp"
#include "../../Scene/InstanceManager.hpp"

namespace mtd
{
    // Handles the objects that should be rendered at each frame
    class RenderObjectManager
    {
        public:
            RenderObjectManager() = default;
            ~RenderObjectManager() = default;

            RenderObjectManager(const RenderObjectManager&) = delete;
            RenderObjectManager& operator=(const RenderObjectManager&) = delete;

            // Getter
            uint32_t getRenderObjectCount() const { return renderObjectCount; }

            // Creates the render objects GPU buffer at the beginning of the scene
            void createBuffer(ResourceManager& resourceManager);

            // Creates the render objects and the draw batches from the scene instances
            void createFrameRenderObjects
            (
                ResourceManager& resourceManager,
                const std::vector<MeshData>& meshes,
                const std::vector<SceneInstance>& sceneInstances,
                std::vector<DrawBatch>& drawBatches,
                DescriptorManager& descriptorManager
            );
            // Updates the descriptor data for the render objects buffer
            void updateDescriptor
            (
                const ResourceManager& resourceManager, DescriptorSetHandler& descriptorSetHandler
            ) const;

            // Binds the render object buffer
            void bindBuffer(const ResourceManager& resourceManager, vk::CommandBuffer commandBuffer) const;

        private:
            // Resource ID for the render objects that will be used in the current frame
            ResourceID renderObjectBufferID = 0U;

            // List of instances visible in the current frame
            std::vector<const SceneInstance*> visibleInstances;
            // List of render objects for the current frame
            std::vector<RenderObject> renderObjects;
            // Count of active render objects in the GPU buffer
            uint32_t renderObjectCount = 0U;

            // Updates the render objects buffer contents
            void updateBufferData(ResourceManager& resourceManager, DescriptorManager& descriptorManager);
    };
}
