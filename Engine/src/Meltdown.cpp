#include <Meltdown.hpp>

#include "Engine.hpp"

mtd::MeltdownEngine::MeltdownEngine() : engine{std::make_unique<Engine>()}
{
}

mtd::MeltdownEngine::~MeltdownEngine() = default;

void mtd::MeltdownEngine::run()
{
	engine->start();
}
