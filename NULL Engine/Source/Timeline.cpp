#include "Timeline.h"

bool ImGui::BeginTimeline(const char* str_id, const float& max_value)
{
	max_timeline_value = max_value;
	return BeginChild(str_id);
}

bool ImGui::TimelineEvent(const char* str_id, float* values)
{
	ImGuiWindow* win = GetCurrentWindow();

	const ImU32 inactive_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_Button]);
	const ImU32 active_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_ButtonHovered]);
	const ImU32 line_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_FrameBgActive]);
	bool changed = false;
	ImVec2 cursor_pos = win->DC.CursorPos;

	for (int i = 0; i < 2; ++i)
	{
		ImVec2 pos = cursor_pos;
		pos.x += (win->Size.x * values[i]) / (max_timeline_value + timeline_radius);
		pos.y += timeline_radius;

		SetCursorScreenPos(pos - ImVec2(timeline_radius, timeline_radius));
		PushID(i);
		InvisibleButton(str_id, ImVec2(2 * timeline_radius, 2 * timeline_radius));
		if (IsItemActive() || IsItemHovered())
		{
			ImGui::SetTooltip("%f", values[i]);
			ImVec2 a(pos.x, GetWindowContentRegionMin().y + win->Pos.y);
			ImVec2 b(pos.x, GetWindowContentRegionMax().y + win->Pos.y);
			win->DrawList->AddLine(a, b, line_color);
		}
		if (IsItemActive() && IsMouseDragging(0))
		{
			values[i] += GetIO().MouseDelta.x / (win->Size.x * max_timeline_value);
			changed = true;
		}
		PopID();
		win->DrawList->AddCircleFilled(pos, timeline_radius, (IsItemActive() || IsItemHovered()) ? active_color : inactive_color);
	}

	ImVec2 start = cursor_pos;
	start.x += (win->Size.x * values[0]) / (max_timeline_value + 2 * timeline_radius);
	start.y += timeline_radius * 0.5f;
	ImVec2 end = start + ImVec2(win->Size.x * ((values[1] - values[0]) / (max_timeline_value - 2 * timeline_radius)), timeline_radius);

	PushID(-1);
	SetCursorScreenPos(start);
	InvisibleButton(str_id, end - start);
	if (IsItemActive() && IsMouseDragging(0))
	{
		values[0] += GetIO().MouseDelta.x / (win->Size.x * max_timeline_value);
		values[1] += GetIO().MouseDelta.x / (win->Size.x * max_timeline_value);

		changed = true;
	}
	PopID();

	SetCursorScreenPos(cursor_pos + ImVec2(0, GetTextLineHeightWithSpacing()));

	win->DrawList->AddRectFilled(start, end, (IsItemActive() || IsItemHovered()) ? active_color : inactive_color);

	if (values[0] > values[1])
	{
		float tmp = values[0];
		values[0] = values[1];
		values[1] = tmp;
	}
	if (values[1] > max_timeline_value)
	{
		values[1] = max_timeline_value;
	}
	if (values[0] < 0)
	{
		values[0] = 0;
	}

	return changed;
}

void ImGui::EndTimeline()
{
	ImGuiWindow* win = GetCurrentWindow();

	ImU32 color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_Button]);
	ImU32 line_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_Border]);
	ImU32 text_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_Text]);
	float rounding = GImGui->Style.ScrollbarRounding;

	ImVec2 start = ImVec2(GetWindowContentRegionMin().x + win->Pos.x, GetWindowContentRegionMax().y - GetTextLineHeightWithSpacing() + win->Pos.y);
	ImVec2 end = GetWindowContentRegionMax() + win->Pos;

	win->DrawList->AddRectFilled(start, end, color, rounding);

	const int LINE_COUNT = 5;
	const ImVec2 text_offset(0, GetTextLineHeightWithSpacing());
	for (int i = 0; i < LINE_COUNT; ++i)
	{
		ImVec2 a = GetWindowContentRegionMin() + win->Pos + ImVec2(timeline_radius, 0);
		a.x += i * GetWindowContentRegionWidth() / LINE_COUNT;
		ImVec2 b = a;
		b.y = start.y;

		win->DrawList->AddLine(a, b, line_color);
		char tmp[256];
		ImFormatString(tmp, sizeof(tmp), "%.2f", i * max_timeline_value / LINE_COUNT);
		win->DrawList->AddText(b, text_color, tmp);
	}

	EndChild();
}

bool ImGui::BeginTimelineEx(const char* str_id, const float& max_value, const int& num_visible_rows, const int& opt_exact_num_rows, ImVec2* popt_offset_and_scale)
{
	// reset global variables
	max_timeline_value = 0.f;
	timeline_num_rows = timeline_display_start = timeline_display_end = 0;
	timeline_display_index = -1;
	ptimeline_offset_and_scale = popt_offset_and_scale;

	if (ptimeline_offset_and_scale) {
		if (ptimeline_offset_and_scale->y == 0.f) { ptimeline_offset_and_scale->y = 1.f; }
	}
	const float row_height = ImGui::GetTextLineHeightWithSpacing();
	const bool rv = BeginChild(str_id, ImVec2(0, num_visible_rows >= 0 ? (row_height * num_visible_rows) : (ImGui::GetContentRegionAvail().y - row_height)), false);
	ImGui::PushStyleColor(ImGuiCol_Separator, GImGui->Style.Colors[ImGuiCol_Border]);
	ImGui::Columns(2, str_id);
	const float contentRegionWidth = ImGui::GetWindowContentRegionWidth();
	if (ImGui::GetColumnOffset(1) >= contentRegionWidth * 0.48f) ImGui::SetColumnOffset(1, contentRegionWidth * 0.15f);
	max_timeline_value = max_value >= 0 ? max_value : (contentRegionWidth * 0.85f);
	if (opt_exact_num_rows > 0) {
		// Item culling
		timeline_num_rows = opt_exact_num_rows;
		ImGui::CalcListClipping(timeline_num_rows, row_height, &timeline_display_start, &timeline_display_end);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (timeline_display_start * row_height));
	}
	return rv;
}

bool ImGui::TimelineEventEx(const char* str_id, float* values, bool keep_range_constant)
{
	++timeline_display_index;
	if (timeline_num_rows > 0 &&
		(timeline_display_index < timeline_display_start || timeline_display_index >= timeline_display_end)) {
		if (timeline_display_index == timeline_display_start - 1) { ImGui::NextColumn(); ImGui::NextColumn(); }    // This fixes a clipping issue at the top visible row
		return false;   // item culling
	}

	const float row_height = ImGui::GetTextLineHeightWithSpacing();
	const float TIMELINE_RADIUS = row_height * 0.45f;
	const float row_height_offset = (row_height - TIMELINE_RADIUS * 2.f) * 0.5f;


	ImGuiWindow* win = GetCurrentWindow();
	const ImU32 inactive_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_Button]);
	const ImU32 active_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_ButtonHovered]);
	const ImU32 line_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_SeparatorActive]);
	bool changed = false;
	bool hovered = false;
	bool active = false;

	ImGui::Text("%s", str_id);
	ImGui::NextColumn();


	const float s_timeline_time_offset = ptimeline_offset_and_scale ? ptimeline_offset_and_scale->x : 0.f;
	const float s_timeline_time_scale = ptimeline_offset_and_scale ? ptimeline_offset_and_scale->y : 1.f;

	const float columnOffset = ImGui::GetColumnOffset(1);
	const float columnWidth = ImGui::GetColumnWidth(1) - GImGui->Style.ScrollbarSize;
	const float columnWidthScaled = columnWidth * s_timeline_time_scale;
	const float columnWidthOffsetScaled = columnWidthScaled * s_timeline_time_offset;
	const ImVec2 cursor_pos(GetWindowContentRegionMin().x + win->Pos.x + columnOffset - TIMELINE_RADIUS, win->DC.CursorPos.y);
	bool mustMoveBothEnds = false;
	const bool isMouseDraggingZero = IsMouseDragging(0);
	float posx[2] = { 0,0 };

	for (int i = 0; i < 2; ++i)
	{
		ImVec2 pos = cursor_pos;
		pos.x += columnWidthScaled * values[i] / max_timeline_value - columnWidthOffsetScaled + TIMELINE_RADIUS;
		pos.y += row_height_offset + TIMELINE_RADIUS;
		posx[i] = pos.x;
		if (pos.x + TIMELINE_RADIUS < cursor_pos.x ||
			pos.x - 2.f * TIMELINE_RADIUS > cursor_pos.x + columnWidth) continue;   // culling

		SetCursorScreenPos(pos - ImVec2(TIMELINE_RADIUS, TIMELINE_RADIUS));
		PushID(i);
		InvisibleButton(str_id, ImVec2(2 * TIMELINE_RADIUS, 2 * TIMELINE_RADIUS));
		active = IsItemActive();
		if (active || IsItemHovered())
		{
			ImGui::SetTooltip("%f", values[i]);
			if (!keep_range_constant) {
				// @meshula:The item hovered line needs to be compensated for vertical scrolling. Thx!
				ImVec2 a(pos.x, GetWindowContentRegionMin().y + win->Pos.y + win->Scroll.y);
				ImVec2 b(pos.x, GetWindowContentRegionMax().y + win->Pos.y + win->Scroll.y);
				// possible aternative:
				//ImVec2 a(pos.x, win->Pos.y);
				//ImVec2 b(pos.x, win->Pos.y+win->Size.y);
				win->DrawList->AddLine(a, b, line_color);
			}
			hovered = true;
		}
		if (active && isMouseDraggingZero)
		{
			if (!keep_range_constant) values[i] += GetIO().MouseDelta.x / columnWidthScaled * max_timeline_value;
			else mustMoveBothEnds = true;
			changed = hovered = true;
		}
		PopID();
		win->DrawList->AddCircleFilled(
			pos, TIMELINE_RADIUS, IsItemActive() || IsItemHovered() ? active_color : inactive_color, 8);
	}

	ImVec2 start(posx[0] + TIMELINE_RADIUS, cursor_pos.y + row_height * 0.3f);
	ImVec2 end(posx[1] - TIMELINE_RADIUS, start.y + row_height * 0.4f);
	if (start.x < cursor_pos.x) start.x = cursor_pos.x;
	if (end.x > cursor_pos.x + columnWidth + TIMELINE_RADIUS) end.x = cursor_pos.x + columnWidth + TIMELINE_RADIUS;
	const bool isInvisibleButtonCulled = start.x > cursor_pos.x + columnWidth || end.x < cursor_pos.x;

	bool isInvisibleButtonItemActive = false;
	bool isInvisibleButtonItemHovered = false;
	if (!isInvisibleButtonCulled) {
		PushID(-1);
		SetCursorScreenPos(start);
		InvisibleButton(str_id, end - start);
		isInvisibleButtonItemActive = IsItemActive();
		isInvisibleButtonItemHovered = isInvisibleButtonItemActive || IsItemHovered();
		PopID();
		win->DrawList->AddRectFilled(start, end, isInvisibleButtonItemActive || isInvisibleButtonItemHovered ? active_color : inactive_color);
	}
	if ((isInvisibleButtonItemActive && isMouseDraggingZero) || mustMoveBothEnds)
	{
		const float deltaX = GetIO().MouseDelta.x / columnWidthScaled * max_timeline_value;
		values[0] += deltaX;
		values[1] += deltaX;
		changed = hovered = true;
	}
	else if (isInvisibleButtonItemHovered) hovered = true;

	SetCursorScreenPos(cursor_pos + ImVec2(0, row_height));

	if (changed) {
		if (values[0] > values[1]) { float tmp = values[0]; values[0] = values[1]; values[1] = tmp; }
		if (values[1] > max_timeline_value) { values[0] -= values[1] - max_timeline_value; values[1] = max_timeline_value; }
		if (values[0] < 0) { values[1] -= values[0]; values[0] = 0; }
	}

	//if (hovered) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

	ImGui::NextColumn();
	return changed;
}

void ImGui::EndTimelineEx(const int& num_vertical_grid_lines, const float& current_time, ImU32 timeline_running_color)
{
	const float row_height = ImGui::GetTextLineHeightWithSpacing();
	if (timeline_num_rows > 0) ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ((timeline_num_rows - timeline_display_end) * row_height));
	ImGui::NextColumn();

	ImGuiWindow* win = GetCurrentWindow();

	const float columnOffset = ImGui::GetColumnOffset(1);
	const float columnWidth = ImGui::GetColumnWidth(1) - GImGui->Style.ScrollbarSize;
	const float s_timeline_time_offset = ptimeline_offset_and_scale ? ptimeline_offset_and_scale->x : 0.f;
	const float s_timeline_time_scale = ptimeline_offset_and_scale ? ptimeline_offset_and_scale->y : 1.f;
	const float columnWidthScaled = columnWidth * s_timeline_time_scale;
	const float columnWidthOffsetScaled = columnWidthScaled * s_timeline_time_offset;
	const float horizontal_interval = columnWidth / num_vertical_grid_lines;

	ImU32 color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_Button]);
	ImU32 line_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_Border]);
	ImU32 text_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_Text]);
	ImU32 moving_line_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_SeparatorActive]);
	const float rounding = GImGui->Style.ScrollbarRounding;
	const float startY = ImGui::GetWindowHeight() + win->Pos.y;

	// Draw black vertical lines (inside scrolling area)
	for (int i = 1; i <= num_vertical_grid_lines; ++i)
	{
		ImVec2 a = GetWindowContentRegionMin() + win->Pos;
		a.x += s_timeline_time_scale * i * horizontal_interval + columnOffset - columnWidthOffsetScaled;
		win->DrawList->AddLine(a, ImVec2(a.x, startY), line_color);
	}

	// Draw moving vertical line
	if (current_time > 0.f && current_time < max_timeline_value) {
		ImVec2 a = GetWindowContentRegionMin() + win->Pos;
		a.x += columnWidthScaled * (current_time / max_timeline_value) + columnOffset - columnWidthOffsetScaled;
		win->DrawList->AddLine(a, ImVec2(a.x, startY), moving_line_color, 3);
	}

	ImGui::Columns(1);
	ImGui::PopStyleColor();

	EndChild();
	const bool isChildWindowHovered = ptimeline_offset_and_scale ? ImGui::IsItemHovered() : false;

	// Draw bottom axis ribbon (outside scrolling region)
	win = GetCurrentWindow();
	float startx = ImGui::GetCursorScreenPos().x + columnOffset;
	float endy = ImGui::GetCursorScreenPos().y + row_height;//GetWindowContentRegionMax().y + win->Pos.y;
	ImVec2 start(startx, ImGui::GetCursorScreenPos().y);
	ImVec2 end(startx + columnWidth, endy);//start.y+row_height);
	float maxx = start.x + columnWidthScaled - columnWidthOffsetScaled;
	if (maxx < end.x) end.x = maxx;
	if (current_time <= 0)			win->DrawList->AddRectFilled(start, end, color, rounding);
	else if (current_time > max_timeline_value) win->DrawList->AddRectFilled(start, end, timeline_running_color, rounding);
	else {
		ImVec2 median(start.x + columnWidthScaled * (current_time / max_timeline_value) - columnWidthOffsetScaled, end.y);
		if (median.x < startx) median.x = startx;
		else {
			if (median.x > startx + columnWidth) median.x = startx + columnWidth;
			win->DrawList->AddRectFilled(start, median, timeline_running_color, rounding, 1 | 8);
		}
		median.y = start.y;
		if (median.x < startx + columnWidth) {
			win->DrawList->AddRectFilled(median, end, color, rounding, 2 | 4);
			if (median.x > startx) win->DrawList->AddLine(median, ImVec2(median.x, end.y), moving_line_color, 3);
		}
	}

	char tmp[256] = "";
	for (int i = 0; i < num_vertical_grid_lines; ++i)
	{
		ImVec2 a = start;
		a.x = start.x + s_timeline_time_scale * i * horizontal_interval - columnWidthOffsetScaled;
		if (a.x < startx || a.x >= startx + columnWidth) continue;

		ImFormatString(tmp, sizeof(tmp), "%.2f", i * max_timeline_value / num_vertical_grid_lines);
		win->DrawList->AddText(a, text_color, tmp);

	}
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + row_height);


	// zoom and pan
	if (ptimeline_offset_and_scale) {
		const ImGuiIO& io = ImGui::GetIO();
		if (isChildWindowHovered && io.KeyCtrl) {
			if (ImGui::IsMouseDragging(1)) {
				// pan
				ptimeline_offset_and_scale->x -= io.MouseDelta.x / columnWidthScaled;
				if (ptimeline_offset_and_scale->x > 1.f) ptimeline_offset_and_scale->x = 1.f;
				else if (ptimeline_offset_and_scale->x < 0.f) ptimeline_offset_and_scale->x = 0.f;
			}
			else if (io.MouseReleased[2]) {
				// reset
				ptimeline_offset_and_scale->x = 0.f;
				ptimeline_offset_and_scale->y = 1.f;
			}
			if (io.MouseWheel != 0) {
				// zoom
				ptimeline_offset_and_scale->y *= (io.MouseWheel > 0) ? 1.05f : 0.95f;
				if (ptimeline_offset_and_scale->y < 0.25f) ptimeline_offset_and_scale->y = 0.25f;
				else if (ptimeline_offset_and_scale->y > 4.f) ptimeline_offset_and_scale->y = 4.f;
			}
		}
	}
}

bool ImGui::PlotTimelineEvents(const char* str_id, float* values, bool keep_range_constant)
{
	bool ret = true;

	++timeline_display_index;
	if (timeline_num_rows > 0 &&
		(timeline_display_index < timeline_display_start || timeline_display_index >= timeline_display_end)) {
		if (timeline_display_index == timeline_display_start - 1) { ImGui::NextColumn(); ImGui::NextColumn(); }    // This fixes a clipping issue at the top visible row
		return false;   // item culling
	}

	const float row_height = ImGui::GetTextLineHeightWithSpacing();
	const float TIMELINE_RADIUS = row_height * 0.45f;
	const float row_height_offset = (row_height - TIMELINE_RADIUS * 2.f) * 0.5f;


	ImGuiWindow* win = GetCurrentWindow();
	const ImU32 inactive_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_Button]);
	const ImU32 active_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_ButtonHovered]);
	const ImU32 line_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_SeparatorActive]);
	bool changed = false;
	bool hovered = false;
	bool active = false;

	ImGui::Text("%s", str_id);
	ImGui::NextColumn();


	const float s_timeline_time_offset = ptimeline_offset_and_scale ? ptimeline_offset_and_scale->x : 0.f;
	const float s_timeline_time_scale = ptimeline_offset_and_scale ? ptimeline_offset_and_scale->y : 1.f;

	const float columnOffset = ImGui::GetColumnOffset(1);
	const float columnWidth = ImGui::GetColumnWidth(1) - GImGui->Style.ScrollbarSize;
	const float columnWidthScaled = columnWidth * s_timeline_time_scale;
	const float columnWidthOffsetScaled = columnWidthScaled * s_timeline_time_offset;
	const ImVec2 cursor_pos(GetWindowContentRegionMin().x + win->Pos.x + columnOffset - TIMELINE_RADIUS, win->DC.CursorPos.y);
	bool mustMoveBothEnds = false;
	const bool isMouseDraggingZero = IsMouseDragging(0);
	float posx[2] = { 0,0 };

	float val[6];
	memcpy(val, values, sizeof(float) * 6);

	for (int j = 0; j < 3; ++j)
	{
		float values[2] = { val[j], val[j + 1] };

		for (int i = 0; i < 2; ++i)
		{
			ImVec2 pos = cursor_pos;
			pos.x += columnWidthScaled * values[i] / max_timeline_value - columnWidthOffsetScaled + TIMELINE_RADIUS;
			pos.y += row_height_offset + TIMELINE_RADIUS;
			posx[i] = pos.x;
			if (pos.x + TIMELINE_RADIUS < cursor_pos.x ||
				pos.x - 2.f * TIMELINE_RADIUS > cursor_pos.x + columnWidth) continue;   // culling

			SetCursorScreenPos(pos - ImVec2(TIMELINE_RADIUS, TIMELINE_RADIUS));
			PushID(i);
			InvisibleButton(str_id, ImVec2(2 * TIMELINE_RADIUS, 2 * TIMELINE_RADIUS));
			active = IsItemActive();
			if (active || IsItemHovered())
			{
				ImGui::SetTooltip("%f", values[i]);
				if (!keep_range_constant) {
					// @meshula:The item hovered line needs to be compensated for vertical scrolling. Thx!
					ImVec2 a(pos.x, GetWindowContentRegionMin().y + win->Pos.y + win->Scroll.y);
					ImVec2 b(pos.x, GetWindowContentRegionMax().y + win->Pos.y + win->Scroll.y);
					// possible aternative:
					//ImVec2 a(pos.x, win->Pos.y);
					//ImVec2 b(pos.x, win->Pos.y+win->Size.y);
					win->DrawList->AddLine(a, b, line_color);
				}
				hovered = true;
			}
			if (active && isMouseDraggingZero)
			{
				if (!keep_range_constant) values[i] += GetIO().MouseDelta.x / columnWidthScaled * max_timeline_value;
				else mustMoveBothEnds = true;
				changed = hovered = true;
			}
			PopID();
			win->DrawList->AddCircleFilled(
				pos, TIMELINE_RADIUS, IsItemActive() || IsItemHovered() ? active_color : inactive_color, 8);
		}
	}

	ImVec2 start(posx[0] + TIMELINE_RADIUS, cursor_pos.y + row_height * 0.3f);
	ImVec2 end(posx[1] - TIMELINE_RADIUS, start.y + row_height * 0.4f);
	if (start.x < cursor_pos.x) start.x = cursor_pos.x;
	if (end.x > cursor_pos.x + columnWidth + TIMELINE_RADIUS) end.x = cursor_pos.x + columnWidth + TIMELINE_RADIUS;
	const bool isInvisibleButtonCulled = start.x > cursor_pos.x + columnWidth || end.x < cursor_pos.x;

	bool isInvisibleButtonItemActive = false;
	bool isInvisibleButtonItemHovered = false;
	if (!isInvisibleButtonCulled) {
		PushID(-1);
		SetCursorScreenPos(start);
		InvisibleButton(str_id, end - start);
		isInvisibleButtonItemActive = IsItemActive();
		isInvisibleButtonItemHovered = isInvisibleButtonItemActive || IsItemHovered();
		PopID();
		win->DrawList->AddRectFilled(start, end, isInvisibleButtonItemActive || isInvisibleButtonItemHovered ? active_color : inactive_color);
	}
	if ((isInvisibleButtonItemActive && isMouseDraggingZero) || mustMoveBothEnds)
	{
		const float deltaX = GetIO().MouseDelta.x / columnWidthScaled * max_timeline_value;
		values[0] += deltaX;
		values[1] += deltaX;
		changed = hovered = true;
	}
	else if (isInvisibleButtonItemHovered) hovered = true;

	SetCursorScreenPos(cursor_pos + ImVec2(0, row_height));

	if (changed) {
		if (values[0] > values[1]) { float tmp = values[0]; values[0] = values[1]; values[1] = tmp; }
		if (values[1] > max_timeline_value) { values[0] -= values[1] - max_timeline_value; values[1] = max_timeline_value; }
		if (values[0] < 0) { values[1] -= values[0]; values[0] = 0; }
	}

	//if (hovered) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

	ImGui::NextColumn();
	return changed;

	return ret;
}