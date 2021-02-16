#include "EditorPanel.h"

EditorPanel::EditorPanel(const char* name, bool is_active) : 
name		(name), 
is_active	(is_active), 
is_hovered	(false),
is_clicked	(false)
{

}

EditorPanel::~EditorPanel()
{

}

bool EditorPanel::Draw(ImGuiIO& io)
{
	bool ret = true;

	return ret;
}

bool EditorPanel::CleanUp()
{
	bool ret = true;

	return ret;
}

// -------- E_PANEL METHODS --------
void EditorPanel::Enable()
{
	if (!is_active)
	{
		is_active = true;
	}
}

void EditorPanel::Disable()
{
	if (is_active)
	{
		is_active = false;
	}
}

bool EditorPanel::IsActive() const
{
	return is_active;
}

bool EditorPanel::IsHovered() const
{
	return is_hovered;
}

bool EditorPanel::IsClicked() const
{
	return is_clicked;
}

void EditorPanel::SetIsHovered()
{	
	if (ImGui::IsWindowHovered())												// Checks if the last generated ImGui window is being hovered.
	{
		is_hovered = true;
	}
	else
	{
		is_hovered = false;
	}
}

void EditorPanel::SetIsClicked(const bool& set_to)
{
	is_clicked = set_to;
}

const char* EditorPanel::GetName() const
{
	return name;
}

void EditorPanel::HelpMarker(const char* description)
{
	ImGui::TextDisabled("(?)");

	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();

		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(description);
		ImGui::PopTextWrapPos();

		ImGui::EndTooltip();
	}
}