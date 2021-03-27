#ifndef __TIMELINE_H__
#define __TIMELINE_H__

#include "ImGui/include/imgui.h"
#include "ImGui/include/imgui_internal.h"

namespace ImGui
{
	// From: { https://github.com/nem0/LumixEngine/blob/timeline_gui/external/imgui/imgui_user.inl#L814 }
	bool BeginTimeline(const char* strId, const float& maxValue);
	bool TimelineEvent(const char* strId, float* values);
	void EndTimeline();

	// From: { https://github.com/bakkesmodorg/PluginManager/blob/master/PluginManager/imgui/imguivariouscontrols.cpp#L3447 }
	bool BeginTimelineEx(const char* strId, const float& maxValue, const int& numVisibleRows, const int& optExactNumRows, ImVec2* poptOffsetAndScale);
	bool TimelineEventEx(const char* strId, float* values, bool keepRangeConstant);
	void EndTimelineEx(const int& numVerticalGridLines, const float& currentTime, ImU32 timelineRunningColor);

	// Own Modifications
	bool PlotTimelineEvents(const char* strId, float* values, bool keepRangeConstant);

	static float		maxTimelineValue;
	static const float	timelineRadius				= 6;

	static int			timelineNumRows			= 0;
	static int			timelineDisplayStart		= 0;
	static int			timelineDisplayEnd		= 0;
	static int			timelineDisplayIndex		= 0;
	static ImVec2*		ptimelineOffsetAndScale	= NULL;
}

#endif // !__TIMELINE_H__