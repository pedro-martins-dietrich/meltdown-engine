#pragma once

#include "../Utils/EngineStructs.hpp"
#include "../Window/Window.hpp"
#include "../Vulkan/Descriptors/DescriptorSetHandler.hpp"

namespace mtd
{
	// Manages camera data
	class Camera
	{
		public:
			Camera(Vec3 initialPosition, float yaw, float pitch, float fovDegrees, float aspectRatio);
			~Camera() = default;

			Camera(const Camera&) = delete;
			Camera& operator=(const Camera&) = delete;

			// Setter
			void setAspectRatio(float newAspectRatio);

			// Updates camera position and direction
			void updateCamera(float deltaTime, const Window& window, DescriptorSetHandler* pGlobalDescriptorSet);

		private:
			// Current camera location
			Vec3 position;
			// Input velocity vector
			Vec3 inputVelocity;
			// View direction angles, in radians
			float yaw;
			float pitch;

			// Camera direction vectors
			Vec3 forwardDirection;
			Vec3 rightDirection;
			Vec3 upDirection;

			// Maximum camera speed
			float maxSpeed;

			// Camera aspect ratio
			float aspectRatio;
			// Camera vertical FOV, in degrees
			float yFOV;
			// Orthographic camera view width
			float viewWidth;
			// Near and far planes
			float nearPlane;
			float farPlane;

			// Orthographic or perspective mode
			bool orthographicMode;

			// Transformation matrices
			CameraMatrices matrices;

			// Updates the view matrix
			void updateViewMatrix();
			// Updates the projection matrix
			void updateProjectionMatrix();

			// Calculates the normalized camera direction vectors
			void calculateDirectionVectors();

			// Sets camera input logic
			void setInputCallbacks();
	};
}
