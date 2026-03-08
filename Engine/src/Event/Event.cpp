#include <pch.hpp>
#include <meltdown/event.hpp>

mtd::KeyPressEvent::KeyPressEvent(KeyCode keyCode, bool repeatedPress)
	: keyCode{keyCode}, repeatedPress{repeatedPress}
{}

mtd::KeyCode mtd::KeyPressEvent::getKeyCode() const
{
	return keyCode;
}

bool mtd::KeyPressEvent::isRepeating() const
{
	return repeatedPress;
}

mtd::KeyReleaseEvent::KeyReleaseEvent(KeyCode keyCode) : keyCode{keyCode}
{}

mtd::KeyCode mtd::KeyReleaseEvent::getKeyCode() const
{
	return keyCode;
}

mtd::MousePositionEvent::MousePositionEvent(float xPos, float yPos, bool cursorHidden)
	: position{xPos, yPos}, cursorHidden{cursorHidden}
{}

const mtd::Vec2& mtd::MousePositionEvent::getMousePosition() const
{
	return position;
}

bool mtd::MousePositionEvent::isCursorHidden() const
{
	return cursorHidden;
}

mtd::ActionStartEvent::ActionStartEvent(uint32_t action) : action{action}
{}

uint32_t mtd::ActionStartEvent::getAction() const
{
	return action;
}

mtd::ActionStopEvent::ActionStopEvent(uint32_t action) : action{action}
{}

uint32_t mtd::ActionStopEvent::getAction() const
{
	return action;
}

mtd::WindowPositionEvent::WindowPositionEvent(int posX, int posY) : posX{posX}, posY{posY}
{}

int mtd::WindowPositionEvent::getPosX() const
{
	return posX;
}

int mtd::WindowPositionEvent::getPosY() const
{
	return posY;
}

mtd::SetPerspectiveCameraEvent::SetPerspectiveCameraEvent(float yFOV, float nearPlane, float farPlane)
	: yFOV{yFOV}, nearPlane{nearPlane}, farPlane{farPlane}
{}

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

mtd::SetOrthographicCameraEvent::SetOrthographicCameraEvent(float viewWidth, float farPlane)
	: viewWidth{viewWidth}, farPlane{farPlane}
{}

float mtd::SetOrthographicCameraEvent::getViewWidth() const
{
	return viewWidth;
}

float mtd::SetOrthographicCameraEvent::getFarPlane() const
{
	return farPlane;
}

mtd::ResetFrameAccumulationEvent::ResetFrameAccumulationEvent()
{}

mtd::ChangeSceneEvent::ChangeSceneEvent(const char* sceneName) : sceneName{sceneName}
{}

const char* mtd::ChangeSceneEvent::getSceneName() const
{
	return sceneName;
}

mtd::CreateInstancesEvent::CreateInstancesEvent(const char* modelID, uint32_t instanceCount)
	: modelID{modelID}, instanceCount{instanceCount}
{}

const std::string& mtd::CreateInstancesEvent::getModelID() const
{
	return modelID;
}

uint32_t mtd::CreateInstancesEvent::getInstanceCount() const
{
	return instanceCount;
}

mtd::RemoveInstanceEvent::RemoveInstanceEvent(uint64_t instanceID) : instanceID{instanceID}
{}

uint64_t mtd::RemoveInstanceEvent::getInstanceID() const
{
	return instanceID;
}

mtd::UpdateDescriptorDataEvent::UpdateDescriptorDataEvent(uint32_t pipelineIndex, uint32_t binding, const void* data)
	: pipelineIndex{pipelineIndex}, binding{binding}, data{data}
{}

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
