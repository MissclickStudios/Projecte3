#ifndef __CURVE_EDITOR_H__
#define __CURVE_EDITOR_H__

#include "ImGui/include/imgui.h"
#include "ImGui/include/imgui_internal.h"

namespace ImGui
{
	// From: { https://github.com/nem0/LumixEngine/blob/39e46c18a58111cc3c8c10a4d5ebbb614f19b1b8/external/imgui/imgui_user.inl#L505-L930 }
	int CurveEditor(const char* label, float* values, int points_count, const ImVec2& editor_size, ImU32 flags, int* new_count);

	enum class STORAGE_VALUES : ImGuiID
	{
		FROM_X = 100,
		FROM_Y,
		WIDTH,
		HEIGHT,
		IS_PANNING,
		POINT_START_X,
		POINT_START_Y
	};

	enum class CURVE_EDITOR_FLAGS
	{
		NONE,
		NO_TANGENTS,
		SHOW_GRID
	};

	static const float node_slot_radius = 4.0f;
}

#endif // !__CURVE_EDITOR_H__