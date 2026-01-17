#include <pch.hpp>
#include <meltdown/window.hpp>

#include "WindowHandler.hpp"

mtd::Window::Window(const WindowInfo& info)
	: windowHandler{std::make_unique<WindowHandler>(info)}
{}

mtd::Window::~Window()
{}

mtd::UIntVec2 mtd::Window::getDimensions() const
{
	return windowHandler->getDimensions();
}

float mtd::Window::getAspectRatio() const
{
	return windowHandler->getAspectRatio();
}
