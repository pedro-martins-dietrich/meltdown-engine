#include <pch.hpp>
#include "InstanceManager.hpp"

#include "../Utils/Logger.hpp"

void mtd::InstanceManager::createInstance(SceneInstance& newInstance)
{
    instances.push_back(newInstance);
}

void mtd::InstanceManager::deleteInstance(uint32_t instanceIndex)
{
    if(instanceIndex < instances.size() - 1)
    {
        instances[instanceIndex] = std::move(instances.back());
        instances.pop_back();
    }
    else if(instanceIndex == instances.size() - 1)
    {
        instances.pop_back();
    }
}

void mtd::InstanceManager::loadInstances(std::vector<SceneInstance>& newInstances)
{
    instances.clear();
    instances = std::move(newInstances);

    LOG_INFO("Loaded %d instances.", instances.size());
}
