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

void mtd::MeltdownEngine::run()
{
	engine->run();
}

void mtd::MeltdownEngine::loadScene(const char* sceneFile)
{
	engine->loadScene(sceneFile);
}

const mtd::Vec3& mtd::CameraHandler::getPosition()
{
	return pCamera->getPosition();
}

const mtd::Vec3& mtd::CameraHandler::getViewDirection()
{
	return pCamera->getViewDirection();
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

void mtd::CameraHandler::setOrientation(float yaw, float pitch)
{
	pCamera->setOrientation(yaw, pitch);
}

void mtd::CameraHandler::translate(const Vec3& deltaPos)
{
	pCamera->translate(deltaPos);
}

void mtd::CameraHandler::rotate(float deltaYaw, float deltaPitch)
{
	pCamera->rotate(deltaYaw, deltaPitch);
}
