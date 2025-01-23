#pragma once

#include "../Utils/EngineStructs.hpp"
#include "../Vulkan/Descriptors/DescriptorSetHandler.hpp"

namespace mtd
{
	// Manages camera data
	class Camera
	{
		public:
			Camera(float aspectRatio);
			~Camera() = default;

			Camera(const Camera&) = delete;
			Camera& operator=(const Camera&) = delete;

			// Getters
			const Vec3& getPosition() const { return position; }
			Vec3 getViewDirection() const { return (orientation * Vec3{0.0f, 0.0f, 1.0f}).normalized(); }
			Vec3 getRightDirection() const { return (orientation * Vec3{1.0f, 0.0f, 0.0f}).normalized(); }
			Vec3 getUpDirection() const { return (orientation * Vec3{0.0f, -1.0f, 0.0f}).normalized(); }
			float getFOV() const { return yFOV; }
			float getNearPlane() const { return nearPlane; }
			float getFarPlane() const { return farPlane; }
			float getViewWidth() const { return viewWidth; }
			bool isOrthographic() const { return orthographicMode; }

			// Setters
			void setAspectRatio(float newAspectRatio);
			void setPosition(const Vec3& newPosition) { position = newPosition; }
			void setOrientation(float newYaw, float newPitch, float newRoll = 0.0f);
			void setOrientation(const Quaternion& newOrientation) { orientation = newOrientation; }

			// Translates the camera position
			void translate(const Vec3& deltaPos) { position += deltaPos; }
			// Rotates the camera orientation by the specified angles, in radians
			void rotate(float deltaYaw, float deltaPitch, float deltaRoll = 0.0f);
			// Applies the rotation described in the quaternion on the camera orientation
			void rotate(const Quaternion& quaternion) { orientation = quaternion * orientation; }

			// Updates the camera matrices
			void updateCamera(DescriptorSetHandler* pGlobalDescriptorSet);

		private:
			// Current camera location
			Vec3 position;
			// Input velocity vector
			Vec3 inputVelocity;
			// Orientation of the camera's internal coordinate system
			Quaternion orientation;

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

			// Sets camera event callbacks
			void setEventCallbacks();
	};
}
