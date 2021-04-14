#include "EngineApplication.h"
#include "Profiler.h"
#include "M_Editor.h"
#include "M_Scene.h"
#include "EditorPanel.h"

#include "E_LoadFile.h"
#include "E_WantToSaveScene.h"

E_WantToSaveScene::E_WantToSaveScene()
{

}

E_WantToSaveScene::~E_WantToSaveScene()
{
}

bool E_WantToSaveScene::Draw(ImGuiIO& io)
{
	OPTICK_CATEGORY("E_WantToSaveScene Draw", Optick::Category::Editor)

	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5, 0.5));
	ImGui::SetNextWindowSize(ImVec2(250.0f, 150.0f));

	ImGui::OpenPopup("Want To Save Scene Popup");

	SetIsHovered();

	if (ImGui::BeginPopupModal("Want To Save Scene Popup", &EngineApp->editor->showWantToSaveScenePopup))
	{
		

		switch (type)
		{
			case WantToSaveType::NEW_SCENE: ImGui::Text("Save current scene before opening New Scene?"); break;
			case WantToSaveType::OPEN_SCENE: ImGui::Text("Save current scene before opening Scene?"); break;
			case WantToSaveType::QUIT: ImGui::Text("Save current scene before Quitting?"); break;
		}

		if (ImGui::Button("Save"))
			Save();

		if (ImGui::Button("Don't Save"))
			DontSave();

		if (ImGui::Button("Cancel"))
			Cancel();

		ImGui::EndPopup();
	}


	return true;
}

bool E_WantToSaveScene::CleanUp()
{
	return false;
}

void E_WantToSaveScene::Save()
{
	switch (type)
	{
		case WantToSaveType::NEW_SCENE:
			EngineApp->scene->SaveCurrentScene();
			EngineApp->scene->NewScene();
			break;
		case WantToSaveType::OPEN_SCENE:
			EngineApp->scene->SaveCurrentScene();
			EngineApp->editor->loadFile->LoadFile();
			break;
		case WantToSaveType::QUIT:

			break;
	}

	EngineApp->editor->showWantToSaveScenePopup = false;
}

void E_WantToSaveScene::DontSave()
{
	switch (type)
	{
		case WantToSaveType::NEW_SCENE:
			EngineApp->scene->NewScene();
			break;
		case WantToSaveType::OPEN_SCENE:
			EngineApp->editor->loadFile->LoadFile();
			break;
		case WantToSaveType::QUIT:

			break;
	}

	EngineApp->editor->showWantToSaveScenePopup = false;
}

void E_WantToSaveScene::Cancel()
{
	EngineApp->editor->showWantToSaveScenePopup = false;
}