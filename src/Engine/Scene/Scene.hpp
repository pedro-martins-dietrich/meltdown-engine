#pragma once

#include "SceneLoader.hpp"

namespace mtd
{
	// Stores the meshes present in the scene
	class Scene
	{
		public:
			Scene(const char* fileName);
			~Scene() {}

			Scene(const Scene&) = delete;
			Scene& operator=(const Scene&) = delete;

			// Getter
			std::vector<Mesh>& getMeshes() { return meshes; }
			Mesh& getMesh(uint32_t index) { return meshes[index]; }

		private:
			// List of meshes used in the scene
			std::vector<Mesh> meshes;
	};
}
