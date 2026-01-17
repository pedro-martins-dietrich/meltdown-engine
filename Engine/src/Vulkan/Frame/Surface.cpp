#include <pch.hpp>
#include "Surface.hpp"

mtd::Surface::Surface(const vk::Instance& instance, const WindowHandler* pWindowHandler)
	: instance{instance}, surface{pWindowHandler->createSurface(instance)}
{}

mtd::Surface::~Surface()
{
	instance.destroySurfaceKHR(surface);
}
