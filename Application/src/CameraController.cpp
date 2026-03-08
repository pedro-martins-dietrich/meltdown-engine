#include "CameraController.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

#include <meltdown/event.hpp>

#include "Actions.hpp"

constexpr float PITCH_LIMIT_RAD = 1.57f;

CameraController::CameraController()
	: inputVelocity{0.0f, 0.0f, 0.0f}, maxSpeed{1.0f},
	lastCursorPos{0.0f, 0.0f}, sensitivity{0.5f, 0.5f}, rollSpeed{0.0f}
{
	registerCallbacks();
}

void CameraController::update(double deltaTime)
{
	mtd::Vec3 velocity{0.0f, 0.0f, 0.0f};
	velocity += mtd::CameraHandler::getRightDirection() * inputVelocity.x;
	velocity += mtd::Vec3{0.0f, -1.0f, 0.0f} * inputVelocity.y;
	velocity += mtd::CameraHandler::getViewDirection() * inputVelocity.z;

	bool cameraMoved = false;

	if(velocity.dot(velocity) > std::numeric_limits<float>::epsilon())
	{
		velocity = velocity.normalized() * maxSpeed;
		mtd::CameraHandler::translate(deltaTime * velocity);
		cameraMoved = true;
	}

	float currentPitch = std::asin(mtd::CameraHandler::getViewDirection().y);
	float newPitch = std::clamp(currentPitch + sensitivity.y * lastCursorPos.y, -PITCH_LIMIT_RAD, PITCH_LIMIT_RAD);
	float deltaPitch = newPitch - currentPitch;

	float deltaYaw = sensitivity.x * lastCursorPos.x;

	if(std::abs(deltaYaw) > std::numeric_limits<float>::epsilon() ||
		std::abs(deltaPitch) > std::numeric_limits<float>::epsilon() ||
		std::abs(rollSpeed) > std::numeric_limits<float>::epsilon())
	{
		mtd::CameraHandler::rotate(deltaYaw, deltaPitch, rollSpeed);
		cameraMoved = true;
	}

	if(cameraMoved)
		mtd::EventManager::dispatch<mtd::ResetFrameAccumulationEvent>();
	lastCursorPos = {0.0f, 0.0f};
}

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
				rollSpeed += 1e-5f;
				break;
			case Actions::RollCW:
				rollSpeed -= 1e-5f;
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
				rollSpeed -= 1e-5f;
				break;
			case Actions::RollCW:
				rollSpeed += 1e-5f;
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
