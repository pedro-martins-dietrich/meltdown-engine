#include "Application.hpp"

#include <array>

#include <meltdown/structs.hpp>
#include <meltdown/model.hpp>

#include "Actions.hpp"
#include "Models/SpinningModel.hpp"
#include "Models/RotatingModel.hpp"

static constexpr std::array<const char*, 2> scenes
{
	"meltdown_demo.json",
	"ray_tracing.json"
};

Application::Application()
	: meltdownEngine{mtd::EngineInfo{"Meltdown Application", 1, 0, 0, true}},
	changeScene{false}
{
	mapActions();
	mtd::ModelHandler::registerModel<SpinningModel>("spinning");
	mtd::ModelHandler::registerModel<RotatingModel>("rotating");

	meltdownEngine.loadScene(scenes[0]);

	meltdownEngine.setClearColor(mtd::Vec4{0.3f, 0.6f, 1.0f, 1.0f});
	meltdownEngine.setVSync(false);

	if(meltdownEngine.isRayTracingEnabled())
	{
		changeSceneCallbackHandle = mtd::EventManager::addCallback([this](const mtd::ActionStartEvent& event)
		{
			if(event.getAction() == Actions::ChangeScene)
				changeScene = true;
		});
	}
}

// Begins the engine main loop
void Application::run()
{
	meltdownEngine.run([this](double deltaTime)
	{
		cameraController.update(deltaTime);
		if(changeScene)
		{
			static uint32_t currentScene = 0U;
			currentScene = (currentScene + 1U) % static_cast<uint32_t>(scenes.size());
			mtd::EventManager::dispatch<mtd::ChangeSceneEvent>(scenes[currentScene]);
			changeScene = false;
		}
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
	mtd::Input::mapAction(Actions::ChangeScene, {KeyCode::LeftControl, KeyCode::R});
}
