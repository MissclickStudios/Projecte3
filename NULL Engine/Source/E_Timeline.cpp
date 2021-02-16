#include "VariableTypedefs.h"

#include "Random.h"
#include "Time.h"

#include "E_Timeline.h"

E_Timeline::E_Timeline() : EditorPanel("Timeline")
{
	current_time = 0.0f;
}

E_Timeline::~E_Timeline()
{

}

bool E_Timeline::Draw(ImGuiIO& io)
{
	bool ret = true;

	static float values[2]	= { 1.0f, 15.0f };
	float values2[2]		= { 15.0f, 30.0f };
	static float values3[2] = { 5.0f, 25.0f };

	//static float values4[6] = { 5.0f, 20.0f, 35.0f, 50.0f, 65.0f, 80.0f };
	static float values4[6]		= { 0.5f, 0.2f, 0.35f, 0.45f, 0.15f, 0.05f };
	static int count			= 6;

	static float values5[10] = {};
	static bool filled = false;

	if (!filled)
	{
		for (uint i = 0; i < 10; ++i)
		{
			values5[i] = Random::LCG::GetBoundedRandomFloat(0.0f, 1.0f);
		}

		filled = true;
	}

	ImGui::Begin("Timeline");

	static ImVec2 offset_and_scale = ImVec2(0.0f, 0.0f);
	ImGui::BeginTimelineEx("Current Animation", 120.0f, 12, 12, &offset_and_scale);
	//ImGui::PlotTimelineEvents("EVENTS", values4, true);
	ImGui::TimelineEventEx("Take001", values, true);
	ImGui::TimelineEventEx("Take002", values2, false);
	ImGui::TimelineEventEx("Transition", values3, false);
	ImGui::EndTimelineEx(12, current_time, ImGui::ColorConvertFloat4ToU32(ImVec4(0.8f, 0.25f, 0.25f, 1.0f)));

	/*ImVec2 curve_editor_width = ImGui::GetWindowSize() * ImVec2(0.75f, 0.85f);
	ImGui::CurveEditor("CURVES", values4, count, curve_editor_width, 2, &count);
	ImGui::CurveEditor("CURVESII", values5, 10, curve_editor_width, 2, nullptr);*/
	
	ImGui::End();

	current_time += Time::Game::GetDT() * 24;																		// Time * Ticks Per Second

	if (current_time > 120.0f)
	{
		current_time = 0.0f;
	}

	return ret;
}

bool E_Timeline::CleanUp()
{
	bool ret = true;

	return ret;
}

// --- IMGUI TIMELINE METHODS ---
