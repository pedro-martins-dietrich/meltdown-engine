#include <pch.hpp>
#include "Profiler.hpp"

#include <chrono>

using ChronoClock = std::chrono::steady_clock;
using ChronoTime = ChronoClock::time_point;
using ChronoDuration = std::chrono::duration<float, std::milli>;

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
	ChronoDuration duration = currentTime - lastStageTime;
	currentFrameData.stageTimes[lastStage] = duration.count();

	lastStage = stage;
	lastStageTime = currentTime;
}

// Ends the last frame stage and also calculates the total frame duration
void mtd::Profiler::endFrame()
{
	ChronoTime currentTime = ChronoClock::now();
	ChronoDuration duration = currentTime - lastStageTime;
	currentFrameData.stageTimes[lastStage] = duration.count();

	ChronoDuration frameDuration = currentTime - initialFrameTime;
	currentFrameData.totalFrameTime = frameDuration.count();

	profiledData = currentFrameData;
	lastStage = nullptr;
}
