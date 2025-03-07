#pragma once

#include <unordered_map>

// Gathers real-time performance information
namespace mtd::Profiler
{
	// Information about the duration of each stage of the frame, as well as the total duration
	struct FrameData
	{
		float totalFrameTime;
		std::unordered_map<const char*, float> stageTimes;
	};

	// Retrieves the data collected by the profiler
	const FrameData& getProfiledData();

	// Begins collecting data about the current frame. The first frame stage must be specified
	void startFrame(const char* initialStage);
	// Ends the last frame stage and starts the next, measuring the time taken
	void nextStage(const char* stage);
	// Ends the last frame stage and also calculates the total frame duration
	void endFrame();

	// Clears the unordered map of frame stages
	void clearStages();
}

#ifdef MTD_DEBUG
	#define PROFILER_START_FRAME(initialStage) Profiler::startFrame(initialStage)
	#define PROFILER_NEXT_STAGE(stage) Profiler::nextStage(stage)
	#define PROFILER_END_FRAME() Profiler::endFrame()
#else
	#define PROFILER_START_FRAME(stage)
	#define PROFILER_NEXT_STAGE(stage)
	#define PROFILER_END_FRAME()
#endif
