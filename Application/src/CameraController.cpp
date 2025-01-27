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
	actionStartCallbackHandle = mtd::EventManager::addCallback([this](const mtd::ActionStartEvent& event)
	{
		switch(event.getAction())
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

	actionStopCallbackHandle = mtd::EventManager::addCallback([this](const mtd::ActionStopEvent& event)
	{
		switch(event.getAction())
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

	mousePositionCallbackHandle = mtd::EventManager::addCallback([this](const mtd::MousePositionEvent& event)
	{
		if(event.isCursorHidden())
			lastCursorPos = event.getMousePosition();
	});
}
