#pragma once

#include <Meltdown.hpp>
#include <meltdown/event.hpp>

// Handles the camera behavior for the application
class CameraController
{
	public:
		CameraController();
		~CameraController() = default;

		CameraController(const CameraController&) = delete;
		CameraController& operator=(const CameraController&) = delete;

		// Updates the camera data every frame
		void update(double deltaTime);

	private:
		// Camera movement data
		mtd::Vec3 inputVelocity;
		float maxSpeed;

		// Camera rotation data
		mtd::Vec2 lastCursorPos;
		const float sensitivityX;
		const float sensitivityY;
		float roll;

		// Callback IDs
		mtd::EventCallbackHandle actionStartCallbackHandle;
		mtd::EventCallbackHandle actionStopCallbackHandle;
		mtd::EventCallbackHandle mousePositionCallbackHandle;

		// Registers action event callbacks for camera input
		void registerCallbacks();
};
