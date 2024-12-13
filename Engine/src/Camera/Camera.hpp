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
			Camera(glm::vec3 initialPosition, float fovDegrees, float aspectRatio);
			~Camera() = default;

			Camera(const Camera&) = delete;
			Camera& operator=(const Camera&) = delete;

			// Getter
			glm::vec3 getPosition() const { return position; }

			// Updates camera position and direction
			void updateCamera(float deltaTime, const Window& window, DescriptorSetHandler* pGlobalDescriptorSet);
			// Updates the perspective matrix
			void updatePerspective(float fovDegrees, float aspectRatio);

		private:
			// Current camera location
			glm::vec3 position;
			// Input velocity vector
			glm::vec3 inputVelocity;
			// View direction angles
			float yaw;
			float pitch;

			// Camera direction vectors
			glm::vec3 forwardDirection;
			glm::vec3 rightDirection;
			glm::vec3 upDirection;
			// Up vector for reference
			const glm::vec3 up;

			// Maximum camera speed
			float maxSpeed;

			// Transformation matrices
			CameraMatrices matrices;

			// Calculates the normalized camera direction vectors
			void calculateDirectionVectors();

			// Sets camera input logic
			void setInputCallbacks();
	};
}
