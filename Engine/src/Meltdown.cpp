#include <pch.hpp>
#include <Meltdown.hpp>

#include "Engine.hpp"

static mtd::Camera* pCamera = nullptr;

mtd::MeltdownEngine::MeltdownEngine(const EngineInfo& applicationInfo)
	: engine{std::make_unique<Engine>(applicationInfo)}
{
	pCamera = &(engine.get()->getCamera());
}

mtd::MeltdownEngine::~MeltdownEngine()
{
	pCamera = nullptr;
}

void mtd::MeltdownEngine::setClearColor(const Vec4& color)
{
	engine->setClearColor(color);
}

void mtd::MeltdownEngine::setVSync(bool enableVSync)
{
	engine->setVSync(enableVSync);
}

void mtd::MeltdownEngine::run(const std::function<void(double)>& onUpdateCallback)
{
	engine->run(onUpdateCallback);
}

void mtd::MeltdownEngine::loadScene(const char* sceneFile)
{
	engine->loadScene(sceneFile);
}

const mtd::Vec3& mtd::CameraHandler::getPosition()
{
	return pCamera->getPosition();
}

mtd::Vec3 mtd::CameraHandler::getViewDirection()
{
	return pCamera->getViewDirection();
}

mtd::Vec3 mtd::CameraHandler::getRightDirection()
{
	return pCamera->getRightDirection();
}

mtd::Vec3 mtd::CameraHandler::getUpDirection()
{
	return pCamera->getUpDirection();
}

float mtd::CameraHandler::getFOV()
{
	return pCamera->getFOV();
}

float mtd::CameraHandler::getNearPlane()
{
	return pCamera->getNearPlane();
}

float mtd::CameraHandler::getFarPlane()
{
	return pCamera->getFarPlane();
}

float mtd::CameraHandler::getViewWidth()
{
	return pCamera->getViewWidth();
}

bool mtd::CameraHandler::isOrthographic()
{
	return pCamera->isOrthographic();
}

void mtd::CameraHandler::setPosition(const Vec3& newPosition)
{
	pCamera->setPosition(newPosition);
}

void mtd::CameraHandler::setOrientation(float yaw, float pitch, float roll)
{
	pCamera->setOrientation(yaw, pitch, roll);
}

void mtd::CameraHandler::setOrientation(const Quaternion& newOrientation)
{
	pCamera->setOrientation(newOrientation);
}

void mtd::CameraHandler::translate(const Vec3& deltaPos)
{
	pCamera->translate(deltaPos);
}

void mtd::CameraHandler::rotate(float deltaYaw, float deltaPitch, float deltaRoll)
{
	pCamera->rotate(deltaYaw, deltaPitch, deltaRoll);
}

void mtd::CameraHandler::rotate(const Quaternion& quaternion)
{
	pCamera->rotate(quaternion);
}
