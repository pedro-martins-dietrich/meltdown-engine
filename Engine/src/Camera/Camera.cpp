#include <pch.hpp>
#include "Camera.hpp"

#include <limits>

#include <meltdown/event.hpp>

#include "../Window/Window.hpp"

static constexpr glm::vec3 up{0.0f, -1.0f, 0.0f};

mtd::Camera::Camera(glm::vec3 initialPosition, float fovDegrees, float aspectRatio)
	: position{initialPosition},
	inputVelocity{0.0f}, maxSpeed{1.0f},
	yaw{0.0f}, pitch{0.5f},
	aspectRatio{aspectRatio},
	yFOV{fovDegrees}, viewWidth{10.0f}, nearPlane{0.01f}, farPlane{1000.0f},
	orthographicMode{false},
	matrices{{1.0f}, {1.0f}, {1.0f}}
{
	calculateDirectionVectors();

	updateViewMatrix();
	updateProjectionMatrix();

	setInputCallbacks();
}

void mtd::Camera::setAspectRatio(float newAspectRatio)
{
	aspectRatio = newAspectRatio;
	updateProjectionMatrix();
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

	updateViewMatrix();
	matrices.projectionView = matrices.projection * matrices.view;

	pGlobalDescriptorSet->updateDescriptorData(0, 0, &matrices, sizeof(CameraMatrices));
}

// Updates the view matrix
void mtd::Camera::updateViewMatrix()
{
	matrices.view = Mat4x4
	{
		rightDirection.x, upDirection.x, -forwardDirection.x, 0.0f,
		rightDirection.y, upDirection.y, -forwardDirection.y, 0.0f,
		rightDirection.z, upDirection.z, -forwardDirection.z, 0.0f,
		-glm::dot(rightDirection, position), -glm::dot(upDirection, position), glm::dot(forwardDirection, position), 1.0f
	};
}

// Updates the projection matrix
void mtd::Camera::updateProjectionMatrix()
{
	if(orthographicMode)
	{
		matrices.projection = Mat4x4
		{
			1.0f / viewWidth, 0.0f, 0.0f, 0.0f,
			0.0, -aspectRatio / viewWidth, 0.0f, 0.0f,
			0.0f, 0.0f, -1.0f / farPlane, 0.0f,	
			0.0f, 0.0f, 0.0f, 1.0f
		};
	}
	else
	{
		const float cotanY = 1.0f / glm::tan(0.5f * glm::radians(yFOV));

		matrices.projection = Mat4x4
		{
			cotanY / aspectRatio, 0.0f, 0.0f, 0.0f,
			0.0f, -cotanY, 0.0f, 0.0f,
			0.0f, 0.0f, farPlane / (nearPlane - farPlane), -1.0f,
			0.0f, 0.0f, (nearPlane * farPlane) / (nearPlane - farPlane), 0.0f
		};
	}
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
	rightDirection = glm::normalize(glm::cross(forwardDirection, up));
	upDirection = glm::cross(rightDirection, forwardDirection);
}

// Sets camera input logic
void mtd::Camera::setInputCallbacks()
{
	EventManager::addCallback(EventType::SetPerspectiveCamera, [this](const Event& e)
	{
		const SetPerspectiveCameraEvent* spce = dynamic_cast<const SetPerspectiveCameraEvent*>(&e);
		if(!spce) return;

		orthographicMode = false;
		yFOV = spce->getFOV();
		nearPlane = spce->getNearPlane();
		farPlane = spce->getFarPlane();
		updateProjectionMatrix();
	});

	EventManager::addCallback(EventType::SetOrthographicCamera, [this](const Event& e)
	{
		const SetOrthographicCameraEvent* soce = dynamic_cast<const SetOrthographicCameraEvent*>(&e);
		if(!soce) return;

		orthographicMode = true;
		viewWidth = soce->getViewWidth();
		farPlane = soce->getFarPlane();
		updateProjectionMatrix();
	});

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
