#pragma once

#include <meltdown/event.hpp>

#include "MeshManager.hpp"

namespace mtd
{
	// Base class for the mesh managers, which implements generic mesh handling
	template<typename MeshType = Mesh>
	class BaseMeshManager : public MeshManager
	{
		static_assert(std::is_base_of_v<Mesh, MeshType>, "The MeshType must be derived from the Mesh class.");

		public:
			BaseMeshManager(const Device& device) : MeshManager{device}
			{
				createInstanceCallbackHandle = EventManager::addCallback([this](const CreateInstancesEvent& event)
				{
					if(meshIndexMap.find(event.getModelID()) == meshIndexMap.end()) return;

					MeshType& mesh = meshes[meshIndexMap.at(event.getModelID())];
					uint32_t instanceVariation = event.getInstanceCount();

					mesh.addInstances(commandHandler, instanceVariation);
				});
				removeInstanceCallbackHandle = EventManager::addCallback([this](const RemoveInstanceEvent& event)
				{
					for(MeshType& mesh: meshes)
						mesh.removeInstanceByID(commandHandler, event.getInstanceID());
				});
			}

			// Getters
			virtual uint32_t getMeshCount() const override { return static_cast<uint32_t>(meshes.size()); }
			std::vector<MeshType>& getMeshes() { return meshes; }

			// Clears mesh data
			virtual void clearMeshes() override
			{
				meshes.clear();
			}

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
			// Event callback handles
			EventCallbackHandle createInstanceCallbackHandle;
			EventCallbackHandle removeInstanceCallbackHandle;
	};
}
