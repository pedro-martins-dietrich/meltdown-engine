#pragma once

#include "../Utils/EngineStructs.hpp"

namespace mtd
{
    // Manager for all instances in the scene
    class InstanceManager
    {
        public:
            InstanceManager() = default;
            ~InstanceManager() = default;

            InstanceManager(const InstanceManager&) = delete;
            InstanceManager& operator=(const InstanceManager&) = delete;

            // Getter
            const std::vector<SceneInstance>& getInstances() const { return instances; }

            // Creates a new scene instance
            void createInstance(SceneInstance& newInstance);
            // Deletes a scene instance
            void deleteInstance(uint32_t instanceIndex);

            // Clears scene instances and loads the initial state of the scene instances
            void loadInstances(std::vector<SceneInstance>& newInstances);

        private:
            // Buffer containing the data from all instances currently in use by the scene
            std::vector<SceneInstance> instances;
    };
}
