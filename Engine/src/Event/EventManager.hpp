#pragma once

namespace mtd::EventManager
{
	// Executes all callbacks related to the queued (dispatched) events.
	void processEvents();
}
