#include <pch.hpp>
#include <Meltdown.hpp>

#include "Engine.hpp"

mtd::MeltdownEngine::MeltdownEngine(const EngineInfo& applicationInfo)
	: engine{std::make_unique<Engine>(applicationInfo)}
{
}

mtd::MeltdownEngine::~MeltdownEngine() = default;

void mtd::MeltdownEngine::run()
{
	engine->run();
}

void mtd::MeltdownEngine::loadScene(const char* sceneFile)
{
	engine->loadScene(sceneFile);
}
