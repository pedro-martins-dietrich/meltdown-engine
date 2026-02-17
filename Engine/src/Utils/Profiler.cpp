#include <pch.hpp>
#include "Profiler.hpp"

#include <Meltdown.hpp>

using ChronoClock = std::chrono::steady_clock;
using ChronoTime = ChronoClock::time_point;
using ChronoDuration = std::chrono::duration<float, std::milli>;

static ChronoTime initialFrameTime;
static ChronoTime lastStageTime;
static const char* lastStage;
static mtd::Profiler::FrameData currentFrameData;
static mtd::Profiler::FrameData profiledData;

const mtd::Profiler::FrameData& mtd::Profiler::getProfiledData()
{
	return profiledData;
}

void mtd::Profiler::startFrame(const char* initialStage)
{
	lastStage = initialStage;
	lastStageTime = ChronoClock::now();
	initialFrameTime = lastStageTime;
}

void mtd::Profiler::nextStage(const char* stage)
{
	ChronoTime currentTime = ChronoClock::now();
	ChronoDuration duration = currentTime - lastStageTime;
	currentFrameData.stageTimes[lastStage] = duration.count();

	lastStage = stage;
	lastStageTime = currentTime;
}

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

void mtd::Profiler::clearStages()
{
	currentFrameData.stageTimes.clear();
	profiledData.stageTimes.clear();
}
