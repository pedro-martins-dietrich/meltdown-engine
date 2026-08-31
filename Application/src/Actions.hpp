#pragma once

#include <cstdint>

// Enumeration for all actions used by the application
enum Actions : uint32_t
{
	Forward,
	Backward,
	Left,
	Right,
	Up,
	Down,
	RollCW,
	RollCCW,
	Run,
	Jump,
	ToggleCameraSettingsGui,
	ToggleProfilerGui,
	ChangeScene,
	Screenshot
};

// Creates a mapping for all actions used by the application
void mapActions();
