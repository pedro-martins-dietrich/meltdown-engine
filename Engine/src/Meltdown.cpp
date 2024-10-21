#include <pch.hpp>
#include <Meltdown.hpp>

#include "Engine.hpp"

mtd::MeltdownEngine::MeltdownEngine(const EngineInfo& applicationInfo)
	: engine{std::make_unique<Engine>(applicationInfo)}
{
}

mtd::MeltdownEngine::~MeltdownEngine() = default;

void mtd::MeltdownEngine::setClearColor(const Vec4& color)
{
	engine->setClearColor(color);
}

void mtd::MeltdownEngine::setVSync(bool enableVSync)
{
	engine->setVSync(enableVSync);
}

void mtd::MeltdownEngine::run()
{
	engine->run();
}

void mtd::MeltdownEngine::loadScene(const char* sceneFile)
{
	engine->loadScene(sceneFile);
}
