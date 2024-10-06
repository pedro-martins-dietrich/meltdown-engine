#include <pch.hpp>
#include "Profiler.hpp"

#include <chrono>

using ChronoClock = std::chrono::high_resolution_clock;
using ChronoTime = std::chrono::steady_clock::time_point;

static ChronoTime initialFrameTime;
static ChronoTime lastStageTime;
static const char* lastStage;
static mtd::Profiler::FrameData currentFrameData;
static mtd::Profiler::FrameData profiledData;

// Retrieves the data collected by the profiler
const mtd::Profiler::FrameData& mtd::Profiler::getProfiledData()
{
	return profiledData;
}

// Begins collecting data about the current frame. The first frame stage must be specified
void mtd::Profiler::startFrame(const char* initialStage)
{
	lastStage = initialStage;
	lastStageTime = ChronoClock::now();
	initialFrameTime = lastStageTime;
}

// Ends the last frame stage and starts the next, measuring the time taken
void mtd::Profiler::nextStage(const char* stage)
{
	ChronoTime currentTime = ChronoClock::now();
	std::chrono::nanoseconds duration = currentTime - lastStageTime;
	currentFrameData.stageTimes[lastStage] = 1.0e-6f * duration.count();

	lastStage = stage;
	lastStageTime = currentTime;
}

// Ends the last frame stage and also calculates the total frame duration
void mtd::Profiler::endFrame()
{
	ChronoTime currentTime = ChronoClock::now();
	std::chrono::nanoseconds duration = currentTime - lastStageTime;
	currentFrameData.stageTimes[lastStage] = 1.0e-6f * duration.count();

	std::chrono::nanoseconds frameDuration = currentTime - initialFrameTime;
	currentFrameData.totalFrameTime = 1.0e-6f * frameDuration.count();

	profiledData = currentFrameData;
	lastStage = nullptr;
}
