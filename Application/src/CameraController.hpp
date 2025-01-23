#pragma once

#include <Meltdown.hpp>

// Handles the camera behavior for the application
class CameraController
{
	public:
		CameraController();
		~CameraController();

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
		uint64_t actionStartCallbackID;
		uint64_t actionStopCallbackID;

		// Registers action event callbacks for camera input
		void registerCallbacks();
};
