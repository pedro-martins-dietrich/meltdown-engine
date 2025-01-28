#include <pch.hpp>
#include <meltdown/event.hpp>

// Key press event
mtd::KeyPressEvent::KeyPressEvent(KeyCode keyCode, bool repeatedPress)
	: keyCode{keyCode}, repeatedPress{repeatedPress}
{
}

mtd::KeyCode mtd::KeyPressEvent::getKeyCode() const
{
	return keyCode;
}

bool mtd::KeyPressEvent::isRepeating() const
{
	return repeatedPress;
}

// Key release event
mtd::KeyReleaseEvent::KeyReleaseEvent(KeyCode keyCode) : keyCode{keyCode}
{
}

mtd::KeyCode mtd::KeyReleaseEvent::getKeyCode() const
{
	return keyCode;
}

// Mouse position event
mtd::MousePositionEvent::MousePositionEvent(float xPos, float yPos, bool cursorHidden)
	: position{xPos, yPos}, cursorHidden{cursorHidden}
{
}

const mtd::Vec2& mtd::MousePositionEvent::getMousePosition() const
{
	return position;
}

bool mtd::MousePositionEvent::isCursorHidden() const
{
	return cursorHidden;
}

// Action start event
mtd::ActionStartEvent::ActionStartEvent(uint32_t action) : action{action}
{
}

uint32_t mtd::ActionStartEvent::getAction() const
{
	return action;
}

//Action stop event
mtd::ActionStopEvent::ActionStopEvent(uint32_t action) : action{action}
{
}

uint32_t mtd::ActionStopEvent::getAction() const
{
	return action;
}

// Window position event
mtd::WindowPositionEvent::WindowPositionEvent(int posX, int posY) : posX{posX}, posY{posY}
{
}

int mtd::WindowPositionEvent::getPosX() const
{
	return posX;
}

int mtd::WindowPositionEvent::getPosY() const
{
	return posY;
}

// Set perspective camera event
mtd::SetPerspectiveCameraEvent::SetPerspectiveCameraEvent(float yFOV, float nearPlane, float farPlane)
	: yFOV{yFOV}, nearPlane{nearPlane}, farPlane{farPlane}
{
}

float mtd::SetPerspectiveCameraEvent::getFOV() const
{
	return yFOV;
}

float mtd::SetPerspectiveCameraEvent::getNearPlane() const
{
	return nearPlane;
}

float mtd::SetPerspectiveCameraEvent::getFarPlane() const
{
	return farPlane;
}

// Set orthographic camera event
mtd::SetOrthographicCameraEvent::SetOrthographicCameraEvent(float viewWidth, float farPlane)
	: viewWidth{viewWidth}, farPlane{farPlane}
{
}

float mtd::SetOrthographicCameraEvent::getViewWidth() const
{
	return viewWidth;
}

float mtd::SetOrthographicCameraEvent::getFarPlane() const
{
	return farPlane;
}

// Change scene event
mtd::ChangeSceneEvent::ChangeSceneEvent(const char* sceneName) : sceneName{sceneName}
{
}

const char* mtd::ChangeSceneEvent::getSceneName() const
{
	return sceneName;
}

// Create instances event
mtd::CreateInstancesEvent::CreateInstancesEvent(const char* modelID, uint32_t instanceCount)
	: modelID{modelID}, instanceCount{instanceCount}
{
}

const std::string& mtd::CreateInstancesEvent::getModelID() const
{
	return modelID;
}

uint32_t mtd::CreateInstancesEvent::getInstanceCount() const
{
	return instanceCount;
}

// Remove instance event
mtd::RemoveInstanceEvent::RemoveInstanceEvent(uint64_t instanceID) : instanceID{instanceID}
{
}

uint64_t mtd::RemoveInstanceEvent::getInstanceID() const
{
	return instanceID;
}

// Update descriptor data event
mtd::UpdateDescriptorDataEvent::UpdateDescriptorDataEvent(uint32_t pipelineIndex, uint32_t binding, const void* data)
	: pipelineIndex{pipelineIndex}, binding{binding}, data{data}
{
}

uint32_t mtd::UpdateDescriptorDataEvent::getPipelineIndex() const
{
	return pipelineIndex;
}

uint32_t mtd::UpdateDescriptorDataEvent::getBinding() const
{
	return binding;
}

const void* mtd::UpdateDescriptorDataEvent::getData() const
{
	return data;
}
