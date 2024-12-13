#include <pch.hpp>
#include "Camera.hpp"

#include <limits>

#include <meltdown/event.hpp>

#include "../Utils/Logger.hpp"
#include "../Window/Window.hpp"

mtd::Camera::Camera(glm::vec3 initialPosition, float fovDegrees, float aspectRatio)
	: position{initialPosition},
	inputVelocity{0.0f},
	maxSpeed{1.0f},
	yaw{0.0f}, pitch{0.0f},
	up{0.0f, -1.0f, 0.0f}
{
	matrices.view = glm::lookAt(position, glm::vec3{0.0f, 0.0f, 0.0f}, up);
	updatePerspective(fovDegrees, aspectRatio);
	calculateDirectionVectors();
	setInputCallbacks();
}

// Updates camera position and direction
void mtd::Camera::updateCamera(float deltaTime, const Window& window, DescriptorSetHandler* pGlobalDescriptorSet)
{
	float mouseX = 0.0f;
	float mouseY = 0.0f;
	window.getMousePos(&mouseX, &mouseY, true);

	yaw += mouseX;
	pitch += mouseY;
	yaw = glm::mod(yaw, glm::two_pi<float>());
	pitch = glm::clamp(pitch, -1.5f, 1.5f);

	calculateDirectionVectors();

	glm::vec3 velocity{0.0f};
	velocity += rightDirection * inputVelocity.x;
	velocity += upDirection * inputVelocity.y;
	velocity += forwardDirection * inputVelocity.z;

	if(glm::length(velocity) > std::numeric_limits<float>::epsilon())
		velocity = glm::normalize(velocity) * maxSpeed;

	position += velocity * deltaTime;

	matrices.view = glm::lookAt(position, position + forwardDirection, up);

	matrices.projectionView = matrices.projection * matrices.view;

	pGlobalDescriptorSet->updateDescriptorData(0, 0, &matrices, sizeof(CameraMatrices));
}

// Updates the perspective matrix
void mtd::Camera::updatePerspective(float fovDegrees, float aspectRatio)
{
	matrices.projection = glm::perspective(glm::radians(fovDegrees), aspectRatio, 0.1f, 100.0f);
	matrices.projection[1][1] *= -1;
}

// Sets camera input logic
void mtd::Camera::setInputCallbacks()
{
	EventManager::addCallback(EventType::KeyPress, [this](const Event& e)
	{
		const KeyPressEvent* keyPress = dynamic_cast<const KeyPressEvent*>(&e);
		if(!keyPress || keyPress->isRepeating()) return;

		switch(keyPress->getKeyCode())
		{
			case KeyCode::W:
				inputVelocity.z += 1.0f;
				break;
			case KeyCode::A:
				inputVelocity.x -= 1.0f;
				break;
			case KeyCode::S:
				inputVelocity.z -= 1.0f;
				break;
			case KeyCode::D:
				inputVelocity.x += 1.0f;
				break;
			case KeyCode::Space:
				inputVelocity.y += 1.0f;
				break;
			case KeyCode::LeftControl:
				inputVelocity.y -= 1.0f;
				break;
			case KeyCode::LeftShift:
				maxSpeed = 2.0f;
				break;
			default:
				return;
		}
	});

	EventManager::addCallback(EventType::KeyRelease, [this](const Event& e)
	{
		const KeyReleaseEvent* keyRelease = dynamic_cast<const KeyReleaseEvent*>(&e);
		if(!keyRelease) return;

		switch(keyRelease->getKeyCode())
		{
			case KeyCode::W:
				inputVelocity.z -= 1.0f;
				break;
			case KeyCode::A:
				inputVelocity.x += 1.0f;
				break;
			case KeyCode::S:
				inputVelocity.z += 1.0f;
				break;
			case KeyCode::D:
				inputVelocity.x -= 1.0f;
				break;
			case KeyCode::Space:
				inputVelocity.y -= 1.0f;
				break;
			case KeyCode::LeftControl:
				inputVelocity.y += 1.0f;
				break;
			case KeyCode::LeftShift:
				maxSpeed = 1.0f;
				break;
			default:
				return;
		}
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
