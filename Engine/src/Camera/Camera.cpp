#include <pch.hpp>
#include "Camera.hpp"

mtd::Camera::Camera(float aspectRatio)
	: position{0.0f, 0.0f, 0.0f},
	inputVelocity{0.0f, 0.0f, 0.0f}, maxSpeed{1.0f},
	orientation{1.0f, 0.0f, 0.0f, 0.0f},
	aspectRatio{aspectRatio},
	yFOV{75.0f}, viewWidth{10.0f}, nearPlane{0.01f}, farPlane{1000.0f},
	orthographicMode{false},
	matrices{{1.0f}, {1.0f}, {1.0f}}
{
	updateViewMatrix();
	updateProjectionMatrix();

	setEventCallbacks();
}

void mtd::Camera::setAspectRatio(float newAspectRatio)
{
	aspectRatio = newAspectRatio;
	updateProjectionMatrix();
}

void mtd::Camera::setOrientation(float newYaw, float newPitch, float newRoll)
{
	Quaternion yawQuat{newYaw, {0.0f, 1.0f, 0.0f}};
	Quaternion pitchQuat{newPitch, {-1.0f, 0.0f, 0.0f}};
	Quaternion rollQuat{newRoll, {0.0f, 0.0f, -1.0f}};
	orientation = (yawQuat * Quaternion{1.0f, 0.0f, 0.0f, 0.0f} * pitchQuat * rollQuat).normalized();
}

// Rotates the camera orientation by the specified angles, in radians
void mtd::Camera::rotate(float deltaYaw, float deltaPitch, float deltaRoll)
{
	Quaternion yawQuat{deltaYaw, {0.0f, 1.0f, 0.0f}};
	Quaternion pitchQuat{deltaPitch, {-1.0f, 0.0f, 0.0f}};
	Quaternion rollQuat{deltaRoll, {0.0f, 0.0f, -1.0f}};
	orientation = (yawQuat * orientation * pitchQuat * rollQuat).normalized();
}

// Updates the camera matrices
void mtd::Camera::updateCamera(DescriptorSetHandler* pGlobalDescriptorSet)
{
	updateViewMatrix();
	pGlobalDescriptorSet->updateDescriptorData(0, 0, &matrices, sizeof(CameraMatrices));
}

// Updates the view matrix
void mtd::Camera::updateViewMatrix()
{
	matrices.view = Mat4x4{orientation};
	matrices.view.w.x = position.dot(Vec3{-matrices.view.x.x, -matrices.view.y.x, -matrices.view.z.x});
	matrices.view.w.y = position.dot(Vec3{-matrices.view.x.y, -matrices.view.y.y, -matrices.view.z.y});
	matrices.view.w.z = position.dot(Vec3{-matrices.view.x.z, -matrices.view.y.z, -matrices.view.z.z});

	matrices.projectionView = matrices.projection * matrices.view;
}

// Updates the projection matrix
void mtd::Camera::updateProjectionMatrix()
{
	if(orthographicMode)
	{
		matrices.projection = Mat4x4
		{
			1.0f / viewWidth, 0.0f, 0.0f, 0.0f,
			0.0, aspectRatio / viewWidth, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f / farPlane, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		};
	}
	else
	{
		const float cotanY = 1.0f / glm::tan(0.5f * glm::radians(yFOV));

		matrices.projection = Mat4x4
		{
			cotanY / aspectRatio, 0.0f, 0.0f, 0.0f,
			0.0f, cotanY, 0.0f, 0.0f,
			0.0f, 0.0f, farPlane / (farPlane - nearPlane), 1.0f,
			0.0f, 0.0f, (nearPlane * farPlane) / (nearPlane - farPlane), 0.0f
		};
	}

	matrices.projectionView = matrices.projection * matrices.view;
}

// Sets camera event callbacks
void mtd::Camera::setEventCallbacks()
{
	setPerspectiveCameraCallbackHandle = EventManager::addCallback([this](const SetPerspectiveCameraEvent& event)
	{
		orthographicMode = false;
		yFOV = event.getFOV();
		nearPlane = event.getNearPlane();
		farPlane = event.getFarPlane();
		updateProjectionMatrix();
	});

	setOrthographicCameraCallbackHandle = EventManager::addCallback([this](const SetOrthographicCameraEvent& event)
	{
		orthographicMode = true;
		viewWidth = event.getViewWidth();
		farPlane = event.getFarPlane();
		updateProjectionMatrix();
	});
}
