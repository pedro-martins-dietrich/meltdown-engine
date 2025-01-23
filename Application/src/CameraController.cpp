#include "CameraController.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

#include <meltdown/event.hpp>

#include "Actions.hpp"

CameraController::CameraController()
	: inputVelocity{0.0f, 0.0f, 0.0f}, maxSpeed{1.0f},
	lastCursorPos{0.0f, 0.0f}, sensitivityX{0.5f}, sensitivityY{0.5f}, roll{0.0f}
{
	registerCallbacks();
}

CameraController::~CameraController()
{
	mtd::EventManager::removeCallback(mtd::EventType::ActionStart, actionStartCallbackID);
	mtd::EventManager::removeCallback(mtd::EventType::ActionStop, actionStopCallbackID);
}

// Updates the camera data every frame
void CameraController::update(double deltaTime)
{
	mtd::Vec3 velocity{0.0f, 0.0f, 0.0f};
	velocity += mtd::CameraHandler::getRightDirection() * inputVelocity.x;
	velocity += mtd::Vec3{0.0f, -1.0f, 0.0f} * inputVelocity.y;
	velocity += mtd::CameraHandler::getViewDirection() * inputVelocity.z;

	if(velocity.dot(velocity) > std::numeric_limits<float>::epsilon())
		velocity = velocity.normalized() * maxSpeed;

	mtd::CameraHandler::translate(deltaTime * velocity);

	float currentPitch = std::asin(mtd::CameraHandler::getViewDirection().y);
	float newPitch = std::clamp(currentPitch + sensitivityY * lastCursorPos.y, -1.57f, 1.57f);

	mtd::CameraHandler::rotate(sensitivityX * lastCursorPos.x, newPitch - currentPitch, roll);
	lastCursorPos = {0.0f, 0.0f};
}

// Registers action event callbacks for camera input
void CameraController::registerCallbacks()
{
	actionStartCallbackID = mtd::EventManager::addCallback(mtd::EventType::ActionStart, [this](const mtd::Event& e)
	{
		const mtd::ActionStartEvent* actionStart = dynamic_cast<const mtd::ActionStartEvent*>(&e);
		if(!actionStart) return;

		switch(actionStart->getAction())
		{
			case Actions::Forward:
				inputVelocity.z += 1.0f;
				break;
			case Actions::Left:
				inputVelocity.x -= 1.0f;
				break;
			case Actions::Backward:
				inputVelocity.z -= 1.0f;
				break;
			case Actions::Right:
				inputVelocity.x += 1.0f;
				break;
			case Actions::Up:
				inputVelocity.y += 1.0f;
				break;
			case Actions::Down:
				inputVelocity.y -= 1.0f;
				break;
			case Actions::Run:
				maxSpeed = 2.0f;
				break;
			case Actions::RollCCW:
				roll += 0.005f;
				break;
			case Actions::RollCW:
				roll -= 0.005f;
				break;
			default:
				return;
		}
	});

	actionStopCallbackID = mtd::EventManager::addCallback(mtd::EventType::ActionStop, [this](const mtd::Event& e)
	{
		const mtd::ActionStopEvent* actionStop = dynamic_cast<const mtd::ActionStopEvent*>(&e);
		if(!actionStop) return;

		switch(actionStop->getAction())
		{
			case Actions::Forward:
				inputVelocity.z -= 1.0f;
				break;
			case Actions::Left:
				inputVelocity.x += 1.0f;
				break;
			case Actions::Backward:
				inputVelocity.z += 1.0f;
				break;
			case Actions::Right:
				inputVelocity.x -= 1.0f;
				break;
			case Actions::Up:
				inputVelocity.y -= 1.0f;
				break;
			case Actions::Down:
				inputVelocity.y += 1.0f;
				break;
			case Actions::Run:
				maxSpeed = 1.0f;
				break;
			case Actions::RollCCW:
				roll -= 0.005f;
				break;
			case Actions::RollCW:
				roll += 0.005f;
				break;
			default:
				return;
		}
	});

	mtd::EventManager::addCallback(mtd::EventType::MousePosition, [this](const mtd::Event& e)
	{
		const mtd::MousePositionEvent* mousePosition = dynamic_cast<const mtd::MousePositionEvent*>(&e);
		if(!mousePosition) return;

		if(mousePosition->isCursorHidden())
			lastCursorPos = mousePosition->getMousePosition();
	});
}
