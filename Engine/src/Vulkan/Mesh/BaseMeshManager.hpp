#pragma once

#include <type_traits>

#include <meltdown/event.hpp>

#include "MeshManager.hpp"

namespace mtd
{
	// Base class for the mesh managers, which implements generic mesh handling
	template <typename MeshType = Mesh>
	class BaseMeshManager : public MeshManager
	{
		static_assert(std::is_base_of<Mesh, MeshType>::value, "The MeshType must be derived from the Mesh class.");

		public:
			BaseMeshManager(const Device& device) : MeshManager{device}
			{
				createInstanceEventCallbackID = EventManager::addCallback
				(
					EventType::CreateInstances,
					[this](const Event& e)
					{
						const CreateInstancesEvent* pEvent = dynamic_cast<const CreateInstancesEvent*>(&e);
						if(meshIndexMap.find(pEvent->getModelID()) == meshIndexMap.end()) return;

						MeshType& mesh = meshes[meshIndexMap.at(pEvent->getModelID())];
						uint32_t instanceVariation = pEvent->getInstanceCount();

						mesh.addInstances(commandHandler, instanceVariation);
					}
				);
				removeInstanceEventCallbackID = EventManager::addCallback
				(
					EventType::RemoveInstance,
					[this](const Event& e)
					{
						const RemoveInstanceEvent* pEvent = dynamic_cast<const RemoveInstanceEvent*>(&e);

						for(MeshType& mesh: meshes)
							mesh.removeInstanceByID(commandHandler, pEvent->getInstanceID());
					}
				);
			}

			~BaseMeshManager()
			{
				EventManager::removeCallback(EventType::CreateInstances, createInstanceEventCallbackID);
				EventManager::removeCallback(EventType::RemoveInstance, removeInstanceEventCallbackID);
			}

			// Getters
			virtual uint32_t getMeshCount() const override { return static_cast<uint32_t>(meshes.size()); }
			std::vector<MeshType>& getMeshes() { return meshes; }

			// Executes the start code for each mesh on scene loading
			virtual void start() override
			{
				for(MeshType& mesh: meshes)
					mesh.start();
			}

			// Updates mesh data
			virtual void update(double frameTime) override
			{
				for(MeshType& mesh: meshes)
					mesh.update(frameTime);
			}

		protected:
			// List of meshes
			std::vector<MeshType> meshes;

			// Map to translate a model ID to a mesh index
			std::unordered_map<std::string, uint32_t> meshIndexMap;

		private:
			// Event callback IDs
			uint64_t createInstanceEventCallbackID;
			uint64_t removeInstanceEventCallbackID;
	};
}
