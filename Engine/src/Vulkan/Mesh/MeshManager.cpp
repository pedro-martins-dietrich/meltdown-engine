#include <pch.hpp>
#include "MeshManager.hpp"

mtd::MeshManager::MeshManager(const Device& device)
	: device{device}, commandHandler{device}
{}
