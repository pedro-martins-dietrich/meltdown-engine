#include "Application.hpp"

#include <meltdown/structs.hpp>
#include <meltdown/model.hpp>

#include "Actions.hpp"
#include "Models/SpinningModel.hpp"
#include "Models/RotatingModel.hpp"

Application::Application()
	: meltdownEngine{mtd::EngineInfo{"Meltdown Application", 1, 0, 0}}
{
	mapActions();
	mtd::ModelHandler::registerModel<SpinningModel>("spinning");
	mtd::ModelHandler::registerModel<RotatingModel>("rotating");

	meltdownEngine.loadScene("meltdown_demo.json");
}

// Begins the engine main loop
void Application::run()
{
	meltdownEngine.run();
}

// Creates a mapping for all actions used by the app
void Application::mapActions()
{
	using mtd::KeyCode;

	mtd::Input::mapAction(Actions::Jump, {KeyCode::LeftShift, KeyCode::Space});
}
