#include "Camera.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include "../Utils/Logger.hpp"
#include "../Window/Window.hpp"

mtd::Camera::Camera
(
	InputHandler& inputHandler,
	glm::vec3 initialPosition,
	float fovDegrees,
	float aspectRatio
) : position{initialPosition}, velocity{0.0f},
	maxSpeed{1.0f},
	yaw{0.0f}, pitch{0.0f},
	up{0.0f, -1.0f, 0.0f},
	frameTime{0.016f}
{
	matrices.view = glm::lookAt
	(
		position,
		glm::vec3{0.0f, 0.0f, 0.0f},
		up
	);
	updatePerspective(fovDegrees, aspectRatio);
	calculateDirectionVectors();
	setInputCallbacks(inputHandler);
}

// Updates camera position and direction
void mtd::Camera::updateCamera(float deltaTime, const Window& window)
{
	frameTime = deltaTime;

	if(glm::length(velocity) > 0.00001f)
		velocity = glm::normalize(velocity) * maxSpeed;
	position += velocity * frameTime;

	float mouseX = 0.0f;
	float mouseY = 0.0f;
	window.getMousePos(&mouseX, &mouseY, true);
	yaw += mouseX;
	pitch += mouseY;
	calculateDirectionVectors();

	matrices.view = glm::lookAt
	(
		position,
		position + forwardDirection,
		up
	);

	velocity = glm::vec3{0.0f};
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

// Sets camera input logic
void mtd::Camera::setInputCallbacks(InputHandler& inputHandler)
{
	inputHandler.setInputCallback("default", "forward", [this](bool pressed)
	{
		if(!pressed) return;
		velocity += forwardDirection;
	});
	inputHandler.setInputCallback("default", "backward", [this](bool pressed)
	{
		if(!pressed) return;
		velocity -= forwardDirection;
	});
	inputHandler.setInputCallback("default", "right", [this](bool pressed)
	{
		if(!pressed) return;
		velocity += rightDirection;
	});
	inputHandler.setInputCallback("default", "left", [this](bool pressed)
	{
		if(!pressed) return;
		velocity -= rightDirection;
	});
	inputHandler.setInputCallback("default", "up", [this](bool pressed)
	{
		if(!pressed) return;
		velocity += up;
	});
	inputHandler.setInputCallback("default", "down", [this](bool pressed)
	{
		if(!pressed) return;
		velocity -= up;
	});
}

// Calculates the normalized camera direction vectors
void mtd::Camera::calculateDirectionVectors()
{
	forwardDirection = glm::vec3
	{
		glm::sin(yaw) * glm::cos(pitch),
		glm::sin(pitch),
		glm::cos(yaw) * glm::cos(pitch)
	};
	rightDirection = glm::cross(forwardDirection, up);
	upDirection = glm::cross(rightDirection, forwardDirection);
}
