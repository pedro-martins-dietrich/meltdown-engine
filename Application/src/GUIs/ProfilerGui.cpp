#include "ProfilerGui.hpp"

#include <Meltdown.hpp>

#include "../Actions.hpp"

#define HOVER_INFO_SIZE 64

ProfilerGui::ProfilerGui(): windowSize{350.0f, 250.0f}, windowPos{20.0f, 450.0f}
{
	toggleGuiCallbackHandle = mtd::EventManager::addCallback([this](const mtd::ActionStartEvent& event)
	{
		if(event.getAction() == Actions::ToggleProfilerGui)
			showWindow = !showWindow;
	});
}

void ProfilerGui::renderGui()
{
	ImGui::SetNextWindowSize(windowSize, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver);
	ImGui::Begin("Frame Profiler", &showWindow);

	profilerGraphic();

	ImGui::End();
}

void ProfilerGui::profilerGraphic()
{
	const mtd::Profiler::FrameData& profileData = mtd::Profiler::getProfiledData();
	const uint32_t stageCount = profileData.stageTimes.size();

	ImGui::Text("Total frame duration: %3.3f ms", profileData.totalFrameTime);
	ImGui::Text("Framerate: %4.1f FPS", 1000.0f / profileData.totalFrameTime);

	ImDrawList* drawList = ImGui::GetWindowDrawList();
	ImVec2 canvasSize = ImGui::GetContentRegionAvail();
	ImVec2 canvasPos = ImGui::GetCursorScreenPos();

	float canvasBottom = canvasPos.y + canvasSize.y - 10.0f;

	float padding = 0.01f * canvasSize.x;
	float offsetX = canvasPos.x + padding;
	float barHeightCoefficient = (canvasSize.y - 10.0f) / profileData.totalFrameTime;
	float barWidth = (canvasSize.x - (stageCount + 2) * padding) / (stageCount + 1);

	for(const auto& [stageName, stageTime]: profileData.stageTimes)
	{
		float barHeight = barHeightCoefficient * stageTime;
		float offsetY = canvasBottom - barHeight;

		ImVec2 rectStart{offsetX, offsetY};
		ImVec2 rectEnd{offsetX + barWidth, offsetY + barHeight};
		bool isRectHovered = ImGui::IsMouseHoveringRect(rectStart, rectEnd);

		drawList->AddRectFilled(rectStart, rectEnd, isRectHovered ? 0xFFFFFF44 : 0xFFFFAA00);
		if(isRectHovered)
		{
			char hoverInfo[HOVER_INFO_SIZE];
			if(std::snprintf(hoverInfo, HOVER_INFO_SIZE, "%s: %.2f ms", stageName, stageTime) > 0)
				drawList->AddText(ImVec2{canvasPos.x, canvasBottom}, 0xFF22CCFF, hoverInfo);
		}

		offsetX += barWidth + padding;
	}

	drawList->AddRectFilled
	(
		ImVec2{offsetX, canvasPos.y}, ImVec2{offsetX + barWidth, canvasBottom}, 0xFFFF0000
	);
}
