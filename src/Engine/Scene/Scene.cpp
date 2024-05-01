#include "Scene.hpp"

mtd::Scene::Scene(const char* fileName)
{
	SceneLoader::load(fileName, meshes);
}
