#include "Actions.hpp"

#include <Meltdown.hpp>

void mapActions()
{
	using mtd::KeyCode;
	using mtd::Input::mapAction;

	mapAction(Actions::Forward, {KeyCode::W});
	mapAction(Actions::Backward, {KeyCode::S});
	mapAction(Actions::Left, {KeyCode::A});
	mapAction(Actions::Right, {KeyCode::D});
	mapAction(Actions::Up, {KeyCode::Space});
	mapAction(Actions::Down, {KeyCode::LeftControl});
	mapAction(Actions::RollCW, {KeyCode::LeftAlt, KeyCode::Period});
	mapAction(Actions::RollCCW, {KeyCode::LeftAlt, KeyCode::Comma});
	mapAction(Actions::Run, {KeyCode::LeftShift});
	mapAction(Actions::Jump, {KeyCode::LeftShift, KeyCode::Space});
	mapAction(Actions::ToggleCameraSettingsGui, {KeyCode::LeftControl, KeyCode::C});
	mapAction(Actions::ToggleProfilerGui, {KeyCode::LeftControl, KeyCode::P});
	mapAction(Actions::ChangeScene, {KeyCode::LeftControl, KeyCode::R});
}
