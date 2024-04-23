#pragma once

#include "../Utils/EngineStructs.hpp"

namespace mtd
{
	// Manages camera data
	class Camera
	{
		public:
			Camera
			(
				glm::vec3 initialPosition,
				glm::vec3 initialVelocity,
				float fovDegrees,
				float aspectRatio
			);
			~Camera() {}

			Camera(const Camera&) = delete;
			Camera& operator=(const Camera&) = delete;

			// Getter
			glm::vec3 getPosition() const { return position; }
			const CameraMatrices* getMatrices() const { return &matrices; }

			// Updates camera position and direction
			void updateCamera(float frameTime, glm::vec3 acceleration);
			// Updates the perspective matrix
			void updatePerspective(float fovDegrees, float aspectRatio);

		private:
			// Current camera location and velocity
			glm::vec3 position;
			glm::vec3 velocity;
			// View direction angles
			float yaw;
			float pitch;

			// Up vector
			const glm::vec3 up;
			// Maximum camera speed
			const float maxSpeed;

			// Transformation matrices
			CameraMatrices matrices;
	};
}
