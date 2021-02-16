#ifndef __THEME_GENERATOR_H__
#define __THEME_GENERATOR_H__

#include "ImGui/include/imgui.h"
#include "ImGui/include/imgui_internal.h"

namespace ImGui
{
	// From: { https://github.com/ocornut/imgui/issues/2265#issuecomment-465432091 }
	void ThemeGenerator(bool is_window = false);

	inline ImVec4 MakeHigh(float alpha);
	inline ImVec4 MakeMid(float alpha);
	inline ImVec4 MakeLow(float alpha);
	inline ImVec4 MakeBg(float alpha, float offset = 0.0f);
	inline ImVec4 MakeText(float alpha);

	static ImVec4		base			= ImVec4(0.502f, 0.075f, 0.256f, 1.0f);
	static ImVec4		bg				= ImVec4(0.200f, 0.220f, 0.270f, 1.0f);
	static ImVec4		text			= ImVec4(0.860f, 0.930f, 0.890f, 1.0f);
	static float		high_val		= 0.8f;
	static float		mid_val			= 0.5f;
	static float		low_val			= 0.3f;
	static float		window_offset	= -0.2f;
}

#endif // !__THEME_GENERATOR_H__