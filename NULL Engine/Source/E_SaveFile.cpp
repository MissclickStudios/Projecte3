#include "E_SaveFile.h"

#include "EngineApplication.h"
#include "FileSystemDefinitions.h"
#include "M_FileSystem.h"
#include "M_Editor.h"
#include "M_Scene.h"


E_SaveFile::E_SaveFile() : EditorPanel("SaveFile", false),
sceneName("MainScene"),
readyToSave(false)
{
}

E_SaveFile::~E_SaveFile()
{
}

bool E_SaveFile::Draw(ImGuiIO& io)
{
	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5, 0.5));
	ImGui::SetNextWindowSize(ImVec2(250.0f, 150.0f));
	
	ImGui::OpenPopup("Save File");

	if (ImGui::BeginPopupModal("Save File"), &EngineApp->editor->showSaveFilePopup, ImGuiWindowFlags_AlwaysAutoResize)
	{
		DrawTextEditor();

		ImGui::EndPopup();
	}

	if (readyToSave)
	{
		readyToSave = false;
		EngineApp->editor->showSaveFilePopup = false;		
	}
	
	return true;
}

bool E_SaveFile::CleanUp()
{
	return false;
}

void E_SaveFile::DrawTextEditor()
{
	ImGui::Spacing();

	if (ImGui::InputText("Scene Name:", (char*)sceneName.c_str(), 64, ImGuiInputTextFlags_EnterReturnsTrue))
	{
		EngineApp->scene->SaveScene(sceneName.c_str());

		ImGui::CloseCurrentPopup();

	}

	ImGui::Spacing();
	ImGui::TextColored(ImVec4(0.6, 0.6, 0.6, 1), "Press 'Enter' to save the scene.");
	ImGui::Spacing();

	if (ImGui::Button("Cancel"))
	{
		readyToSave = false;
		EngineApp->editor->showSaveFilePopup = false;
		ImGui::CloseCurrentPopup();
	}
}
