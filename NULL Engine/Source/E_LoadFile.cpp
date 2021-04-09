#include <vector>
#include <algorithm>

#include "Macros.h"
#include "Profiler.h"

#include "EngineApplication.h"
#include "FileSystemDefinitions.h"
#include "M_FileSystem.h"
#include "M_Editor.h"

#include "E_LoadFile.h"
#include "E_WantToSaveScene.h"

#include "MemoryManager.h"

#define MAX_FILE_SIZE 500

E_LoadFile::E_LoadFile() : EditorPanel("LoadFile", false),
selectedFile	(nullptr),
readyToLoad	(false)
{
	selectedFile = new char[MAX_FILE_SIZE];
	selectedFile[0] = '\0';
}

E_LoadFile::~E_LoadFile()
{

}

bool E_LoadFile::Draw(ImGuiIO& io)
{
	bool ret = true;
	OPTICK_CATEGORY("E_LoadFile Draw", Optick::Category::Editor)
	
	ImGui::OpenPopup("Load File");

	if (ImGui::BeginPopupModal("Load File"), &EngineApp->editor->showLoadFilePopup, ImGuiWindowFlags_AlwaysAutoResize)
	{
		DrawFileBrowser();

		DrawFileSelector();

		ImGui::EndPopup();
	}

	if (readyToLoad)
	{
		EngineApp->editor->OpenWantToSaveScenePopup(WantToSaveType::OPEN_SCENE);
		readyToLoad = false;
		EngineApp->editor->showLoadFilePopup = false;
	}

	return ret;
}

bool E_LoadFile::CleanUp()
{
	bool ret = true;

	RELEASE_ARRAY(selectedFile);

	return ret;
}

void E_LoadFile::OnOpenPopup()
{
	UpdateDirectoryTree(ASSETS_SCENES_PATH);
}

// --- E_LOADFILE METHODS ---
void E_LoadFile::DrawFileBrowser()
{
	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
	ImGui::BeginChild("Asset Files", ImVec2(0, 300), true);

	if (ImGui::TreeNodeEx(ASSETS_SCENES_PATH, ImGuiTreeNodeFlags_DefaultOpen))
	{
		DrawDirectoriesTree(ASSETS_SCENES_PATH, DOTLESS_META_EXTENSION);
		ImGui::TreePop();
	}

	ImGui::EndChild();
	ImGui::PopStyleVar();
}

void E_LoadFile::DrawFileSelector()
{
	ImGui::PushItemWidth(250.0f);
	if (ImGui::InputText("##file_selector", selectedFile, MAX_FILE_SIZE, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
	{
		readyToLoad = true;
	}
	ImGui::PopItemWidth();

	ImGui::SameLine();

	if (ImGui::Button("Confirm"))
	{
		readyToLoad = true;
	}

	ImGui::SameLine();

	if (ImGui::Button("Cancel"))
	{
		selectedFile[0] = '\0';

		readyToLoad = false;
		EngineApp->editor->showLoadFilePopup = false;
	}
}

void E_LoadFile::DrawDirectoriesTree(const char* rootDirectory, const char* extensionToFilter)
{

	for (uint i = 0; i < sceneFiles.size(); ++i)
	{
		if (ImGui::TreeNodeEx(sceneFiles[i].c_str(), ImGuiTreeNodeFlags_Leaf))
		{
			if (ImGui::IsItemClicked())
			{
				sprintf_s(selectedFile, MAX_FILE_SIZE, "%s%s", rootDirectory, sceneFiles[i].c_str());

				if (ImGui::IsMouseDoubleClicked(0))
				{
					readyToLoad = true;
					ImGui::TreePop();
					return;
				}
			}

			ImGui::TreePop();
		}
	}
}

void E_LoadFile::UpdateDirectoryTree(const char* rootDirectory)
{
	sceneFiles.clear();

	std::string rootDir = rootDirectory;

	EngineApp->fileSystem->DiscoverFiles(rootDir.c_str(), sceneFiles, directories);

	std::sort(sceneFiles.begin(), sceneFiles.end());
}

void E_LoadFile::LoadFile()
{
	EngineApp->editor->LoadFileThroughEditor(selectedFile);
	selectedFile[0] = '\0';
}


