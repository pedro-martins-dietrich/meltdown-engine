#include "Application.hpp"

#include <meltdown/structs.hpp>
#include <meltdown/model.hpp>

#include "Actions.hpp"
#include "Models/SpinningModel.hpp"
#include "Models/RotatingModel.hpp"

Application::Application()
	: window{mtd::WindowInfo{1280U, 720U, 640, 360, "Meltdown"}},
	meltdownEngine{mtd::EngineInfo{"Meltdown Application", 1U, 0U, 0U, true}, window}
{
	mapActions();
	mtd::ModelHandler::registerModel<SpinningModel>("spinning");
	mtd::ModelHandler::registerModel<RotatingModel>("rotating");

	meltdownEngine.initializeGui(window);
	meltdownEngine.addGuiWindow(&cameraSettingsGui);
	meltdownEngine.addGuiWindow(&profilerGui);

	scenes =
	{
		"meltdown_demo.json",
		"cellular_automaton.json"
	};

	if(meltdownEngine.isRayTracingEnabled())
		scenes.push_back("ray_tracing.json");

	meltdownEngine.loadScene(scenes[currentSceneIndex]);

	meltdownEngine.setClearColor(mtd::Vec4{0.3f, 0.6f, 1.0f, 1.0f});
	meltdownEngine.setVSync(false);

	changeSceneCallbackHandle = mtd::EventManager::addCallback([this](const mtd::ActionStartEvent& event)
	{
		if(event.getAction() == Actions::ChangeScene)
			changeScene = true;
	});
}

static mtd::Vec4 lightData{0.6f, 0.8f, 0.0f, 0.2f};

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
				currentSceneIndex = (currentSceneIndex + 1U) % scenes.size();
				mtd::EventManager::dispatch<mtd::ChangeSceneEvent>(scenes[currentSceneIndex]);
				changeScene = false;
			}

			if(currentSceneIndex == 0U)
			{
				lightData += deltaTime * mtd::Vec4{-lightData.z, 0.0f, lightData.x, 0.0f};
				mtd::EventManager::dispatch<mtd::UpdateGpuBufferEvent>("LightData", sizeof(LightData), &lightData);
			}
		}
	);
}
