#ifndef __TIMELINE_H__
#define __TIMELINE_H__

#include "ImGui/include/imgui.h"
#include "ImGui/include/imgui_internal.h"

namespace ImGui
{
	// From: { https://github.com/nem0/LumixEngine/blob/timeline_gui/external/imgui/imgui_user.inl#L814 }
	bool BeginTimeline(const char* str_id, const float& max_value);
	bool TimelineEvent(const char* str_id, float* values);
	void EndTimeline();

	// From: { https://github.com/bakkesmodorg/PluginManager/blob/master/PluginManager/imgui/imguivariouscontrols.cpp#L3447 }
	bool BeginTimelineEx(const char* str_id, const float& max_value, const int& num_visible_rows, const int& opt_exact_num_rows, ImVec2* popt_offset_and_scale);
	bool TimelineEventEx(const char* str_id, float* values, bool keep_range_constant);
	void EndTimelineEx(const int& num_vertical_grid_lines, const float& current_time, ImU32 timeline_running_color);

	// Own Modifications
	bool PlotTimelineEvents(const char* str_id, float* values, bool keep_range_constant);

	static float		max_timeline_value;
	static const float	timeline_radius				= 6;

	static int			timeline_num_rows			= 0;
	static int			timeline_display_start		= 0;
	static int			timeline_display_end		= 0;
	static int			timeline_display_index		= 0;
	static ImVec2*		ptimeline_offset_and_scale	= NULL;
}

#endif // !__TIMELINE_H__