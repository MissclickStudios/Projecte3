#include "EditorPanel.h"

EditorPanel::EditorPanel(const char* name, bool isActive) : 
name		(name), 
isActive	(isActive), 
isHovered	(false),
isClicked	(false)
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
	if (!isActive)
	{
		isActive = true;
	}
}

void EditorPanel::Disable()
{
	if (isActive)
	{
		isActive = false;
	}
}

bool EditorPanel::IsActive() const
{
	return isActive;
}

bool EditorPanel::IsHovered() const
{
	return isHovered;
}

bool EditorPanel::IsClicked() const
{
	return isClicked;
}

void EditorPanel::SetIsHovered()
{	
	if (ImGui::IsWindowHovered())												// Checks if the last generated ImGui window is being hovered.
	{
		isHovered = true;
	}
	else
	{
		isHovered = false;
	}
}

void EditorPanel::SetIsClicked(const bool& setTo)
{
	isClicked = setTo;
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