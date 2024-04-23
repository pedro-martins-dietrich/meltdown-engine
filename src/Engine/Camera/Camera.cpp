#include "Camera.hpp"

#include <glm/gtc/matrix_transform.hpp>

mtd::Camera::Camera
(
	glm::vec3 initialPosition,
	glm::vec3 initialVelocity,
	float fovDegrees,
	float aspectRatio
) : position{initialPosition},
	velocity{initialVelocity},
	up{0.0f, -1.0f, 0.0f},
	maxSpeed{2.0f},
	yaw{0.0f},
	pitch{0.0f}
{
	matrices.view = glm::lookAt
	(
		position,
		glm::vec3{0.0f, 0.0f, 0.0f},
		up
	);
	updatePerspective(fovDegrees, aspectRatio);
}

// Updates camera position and direction
void mtd::Camera::updateCamera(float frameTime, glm::vec3 acceleration)
{
	velocity += acceleration * frameTime;
	float speed = glm::length(velocity);
	if(speed > maxSpeed)
		velocity *= (speed / maxSpeed);

	position += velocity * frameTime;

	glm::vec3 viewDirection{glm::sin(yaw), -glm::sin(pitch), -glm::cos(yaw) * glm::cos(pitch)};
	matrices.view = glm::lookAt
	(
		position,
		glm::vec3{0.0f, 0.0f, 0.0f},
		up
	);
}

// Updates the perspective matrix
void mtd::Camera::updatePerspective(float fovDegrees, float aspectRatio)
{
	matrices.projection = glm::perspective
	(
		glm::radians(fovDegrees),
		aspectRatio,
		0.1f,
		100.0f
	);
	matrices.projection[1][1] *= -1;
}
