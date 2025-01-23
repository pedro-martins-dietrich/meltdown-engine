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

	meltdownEngine.setClearColor(mtd::Vec4{0.3f, 0.6f, 1.0f, 1.0f});
	meltdownEngine.setVSync(false);
}

// Begins the engine main loop
void Application::run()
{
	meltdownEngine.run([this](double deltaTime)
	{
		cameraController.update(deltaTime);
	});
}

// Creates a mapping for all actions used by the app
void Application::mapActions()
{
	using mtd::KeyCode;

	mtd::Input::mapAction(Actions::Forward, {KeyCode::W});
	mtd::Input::mapAction(Actions::Backward, {KeyCode::S});
	mtd::Input::mapAction(Actions::Left, {KeyCode::A});
	mtd::Input::mapAction(Actions::Right, {KeyCode::D});
	mtd::Input::mapAction(Actions::Up, {KeyCode::Space});
	mtd::Input::mapAction(Actions::Down, {KeyCode::LeftControl});
	mtd::Input::mapAction(Actions::RollCW, {KeyCode::LeftAlt, KeyCode::Period});
	mtd::Input::mapAction(Actions::RollCCW, {KeyCode::LeftAlt, KeyCode::Comma});
	mtd::Input::mapAction(Actions::Run, {KeyCode::LeftShift});
	mtd::Input::mapAction(Actions::Jump, {KeyCode::LeftShift, KeyCode::Space});
}
