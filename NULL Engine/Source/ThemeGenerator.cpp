#include "ThemeGenerator.h"

void ImGui::ThemeGenerator(bool is_window)
{
	if (is_window)
	{
		Begin("Theme Generator");
	}

    ColorEdit3("base", (float*) &base, ImGuiColorEditFlags_PickerHueWheel);
    ColorEdit3("bg", (float*) &bg, ImGuiColorEditFlags_PickerHueWheel);
    ColorEdit3("text", (float*) &text, ImGuiColorEditFlags_PickerHueWheel);
    SliderFloat("high", &high_val, 0, 1);
    SliderFloat("mid", &mid_val, 0, 1);
    SliderFloat("low", &low_val, 0, 1);
    SliderFloat("window", &window_offset, -0.4f, 0.4f);

    ImGuiStyle &style = ImGui::GetStyle();

    style.Colors[ImGuiCol_Text]                  = MakeText(0.78f);
    style.Colors[ImGuiCol_TextDisabled]          = MakeText(0.28f);
    style.Colors[ImGuiCol_WindowBg]              = MakeBg(1.00f, window_offset);
    style.Colors[ImGuiCol_ChildBg]               = MakeBg(0.58f);
    style.Colors[ImGuiCol_PopupBg]               = MakeBg(0.9f);
    style.Colors[ImGuiCol_Border]                = MakeBg(0.6f, -0.05f);
    style.Colors[ImGuiCol_BorderShadow]          = MakeBg(0.0f, 0.0f);
    style.Colors[ImGuiCol_FrameBg]               = MakeBg(1.00f);
    style.Colors[ImGuiCol_FrameBgHovered]        = MakeMid(0.78f);
    style.Colors[ImGuiCol_FrameBgActive]         = MakeMid(1.00f);
    style.Colors[ImGuiCol_TitleBg]               = MakeLow(1.00f);
    style.Colors[ImGuiCol_TitleBgActive]         = MakeHigh(1.00f);
    style.Colors[ImGuiCol_TitleBgCollapsed]      = MakeBg(0.75f);
    style.Colors[ImGuiCol_MenuBarBg]             = MakeBg(0.47f);
    style.Colors[ImGuiCol_ScrollbarBg]           = MakeBg(1.00f);
    style.Colors[ImGuiCol_ScrollbarGrab]         = MakeLow(1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered]  = MakeMid(0.78f);
    style.Colors[ImGuiCol_ScrollbarGrabActive]   = MakeMid(1.00f);
    style.Colors[ImGuiCol_CheckMark]             = MakeHigh(1.00f);
    style.Colors[ImGuiCol_SliderGrab]            = MakeBg(1.0f, .1f);
    style.Colors[ImGuiCol_SliderGrabActive]      = MakeHigh(1.0f);
    style.Colors[ImGuiCol_Button]                = MakeBg(1.0f, .2f);
    style.Colors[ImGuiCol_ButtonHovered]         = MakeMid(1.00f);
    style.Colors[ImGuiCol_ButtonActive]          = MakeHigh(1.00f);
    style.Colors[ImGuiCol_Header]                = MakeMid(0.76f);
    style.Colors[ImGuiCol_HeaderHovered]         = MakeMid(0.86f);
    style.Colors[ImGuiCol_HeaderActive]          = MakeHigh(1.00f);
    style.Colors[ImGuiCol_ResizeGrip]            = ImVec4(0.47f, 0.77f, 0.83f, 0.04f);
    style.Colors[ImGuiCol_ResizeGripHovered]     = MakeMid(0.78f);
    style.Colors[ImGuiCol_ResizeGripActive]      = MakeMid(1.00f);
    style.Colors[ImGuiCol_PlotLines]             = MakeText(0.63f);
    style.Colors[ImGuiCol_PlotLinesHovered]      = MakeMid(1.00f);
    style.Colors[ImGuiCol_PlotHistogram]         = MakeText(0.63f);
    style.Colors[ImGuiCol_PlotHistogramHovered]  = MakeMid(1.00f);
    style.Colors[ImGuiCol_TextSelectedBg]        = MakeMid(0.43f);
    style.Colors[ImGuiCol_ModalWindowDimBg]      = MakeBg(0.73f);
    style.Colors[ImGuiCol_Tab]                   = MakeBg(0.40f);
    style.Colors[ImGuiCol_TabHovered]            = MakeHigh(1.00f);
    style.Colors[ImGuiCol_TabActive]             = MakeMid(1.00f);
    style.Colors[ImGuiCol_TabUnfocused]          = MakeBg(0.40f);
    style.Colors[ImGuiCol_TabUnfocusedActive]    = MakeBg(0.70f);
    style.Colors[ImGuiCol_DockingPreview]        = MakeHigh(0.30f);

    if (ImGui::Button("Export")) 
	{
      ImGui::LogToTTY();
      ImGui::LogText("ImVec4* colors = ImGui::GetStyle().Colors;\n");
      
	  for (int i = 0; i < ImGuiCol_COUNT; i++) 
	  {
        const ImVec4& col	= style.Colors[i];
        const char* name	= ImGui::GetStyleColorName(i);
        ImGui::LogText("colors[ImGuiCol_%s]%*s= ImVec4(%.2ff, %.2ff, %.2ff, %.2ff);\n", name, 23 - (int)strlen(name), "", col.x, col.y, col.z, col.w);
      }

      ImGui::LogFinish();
    }

	if (is_window)
	{
		ImGui::End();
	}
}

ImVec4 ImGui::MakeHigh(float alpha)
{
	ImVec4 res(0, 0, 0, alpha);
	ColorConvertRGBtoHSV(base.x, base.y, base.z, res.x, res.y, res.z);
	res.z = high_val;
	ColorConvertHSVtoRGB(res.x, res.y, res.z, res.x, res.y, res.z);
	
	return res;
}

ImVec4 ImGui::MakeMid(float alpha)
{
	ImVec4 res(0, 0, 0, alpha);
	ImGui::ColorConvertRGBtoHSV(base.x, base.y, base.z, res.x, res.y, res.z);
	res.z = mid_val;
	ImGui::ColorConvertHSVtoRGB(res.x, res.y, res.z, res.x, res.y, res.z);

	return res;
}

ImVec4 ImGui::MakeLow(float alpha)
{
	ImVec4 res(0, 0, 0, alpha);
	ImGui::ColorConvertRGBtoHSV(base.x, base.y, base.z, res.x, res.y, res.z);
	res.z = low_val;
	ImGui::ColorConvertHSVtoRGB(res.x, res.y, res.z, res.x, res.y, res.z);

	return res;
}

ImVec4 ImGui::MakeBg(float alpha, float offset)
{
	ImVec4 res(0, 0, 0, alpha);
	ImGui::ColorConvertRGBtoHSV(bg.x, bg.y, bg.z, res.x, res.y, res.z);
	res.z += offset;
	ImGui::ColorConvertHSVtoRGB(res.x, res.y, res.z, res.x, res.y, res.z);

	return res;
}

ImVec4 ImGui::MakeText(float alpha)
{
	return ImVec4(text.x, text.y, text.z, alpha);
}