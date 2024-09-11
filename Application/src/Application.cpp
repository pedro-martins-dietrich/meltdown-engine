#include "Application.hpp"

#include <meltdown/structs.hpp>
#include <meltdown/model.hpp>

#include "Models/SpinningModel.hpp"
#include "Models/RotatingModel.hpp"

Application::Application()
	: meltdownEngine{mtd::EngineInfo{"Meltdown Application", 1, 0, 0}}
{
	mtd::ModelHandler::registerModel<SpinningModel>("spinning");
	mtd::ModelHandler::registerModel<RotatingModel>("rotating");

	meltdownEngine.loadScene("meltdown_demo.json");
}

// Begins the engine main loop
void Application::run()
{
	meltdownEngine.run();
}
