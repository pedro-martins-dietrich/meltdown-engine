#pragma once

#include "../Utils/EngineStructs.hpp"
#include "../Input/InputHandler.hpp"

namespace mtd
{
	// Manages camera data
	class Camera
	{
		public:
			Camera
			(
				InputHandler& inputHandler,
				glm::vec3 initialPosition,
				float fovDegrees,
				float aspectRatio
			);
			~Camera() {}

			Camera(const Camera&) = delete;
			Camera& operator=(const Camera&) = delete;

			// Getter
			glm::vec3 getPosition() const { return position; }
			const CameraMatrices* getMatrices() const { return &matrices; }

			// Updates the frame time
			void setFrameTime(float time) { frameTime = time; }

			// Updates camera position and direction
			void updateCamera();
			// Updates the perspective matrix
			void updatePerspective(float fovDegrees, float aspectRatio);

		private:
			// Current camera location
			glm::vec3 position;
			glm::vec3 velocity;
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
			const float maxSpeed;

			// Frame time
			float frameTime;

			// Transformation matrices
			CameraMatrices matrices;

			// Calculates the normalized camera direction vectors
			void calculateDirectionVectors();

			// Sets camera input logic
			void setInputCallbacks(InputHandler& inputHandler);
	};
}
