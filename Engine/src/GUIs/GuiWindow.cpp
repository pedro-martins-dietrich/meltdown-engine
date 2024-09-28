#include <pch.hpp>
#include "GuiWindow.hpp"

mtd::GuiWindow::GuiWindow(ImVec2 winSize, ImVec2 winPos)
	: showWindow{false}, windowSize{winSize}, windowPos{winPos}
{
}
