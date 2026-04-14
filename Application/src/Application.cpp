#include "Application.hpp"

#include <array>

#include <meltdown/structs.hpp>
#include <meltdown/model.hpp>

#include "Actions.hpp"
#include "Models/SpinningModel.hpp"
#include "Models/RotatingModel.hpp"

static constexpr std::array<const char*, 3> scenes
{
	"meltdown_demo.json",
	"ray_tracing.json",
	"cellular_automaton.json"
};

Application::Application()
	: window{mtd::WindowInfo{1280, 720, 640, 360, "Meltdown"}},
	meltdownEngine{mtd::EngineInfo{"Meltdown Application", 1, 0, 0, true}, window},
	cameraSettingsGui{}, profilerGui{}
{
	mapActions();
	mtd::ModelHandler::registerModel<SpinningModel>("spinning");
	mtd::ModelHandler::registerModel<RotatingModel>("rotating");

	meltdownEngine.initializeGui(window);
	meltdownEngine.addGuiWindow(&cameraSettingsGui);
	meltdownEngine.addGuiWindow(&profilerGui);

	meltdownEngine.loadScene(scenes[currentSceneIndex]);

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

void Application::run()
{
	meltdownEngine.run
	(
		window,
		[this](double deltaTime)
		{
			cameraController.update(deltaTime);
			if(changeScene)
			{
				currentSceneIndex = (currentSceneIndex + 1U) % static_cast<uint32_t>(scenes.size());
				mtd::EventManager::dispatch<mtd::ChangeSceneEvent>(scenes[currentSceneIndex]);
				changeScene = false;
			}
		}
	);
}
