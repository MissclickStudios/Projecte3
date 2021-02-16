#include <vector>
#include <algorithm>

#include "Macros.h"

#include "Application.h"
#include "M_FileSystem.h"
#include "M_Editor.h"

#include "E_LoadFile.h"

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
	
	ImGui::OpenPopup("Load File");

	if (ImGui::BeginPopupModal("Load File"), &App->editor->showLoadFilePopup, ImGuiWindowFlags_AlwaysAutoResize)
	{
		DrawFileBrowser();

		DrawFileSelector();

		ImGui::EndPopup();
	}

	if (readyToLoad)
	{
		readyToLoad						= false;
		App->editor->showLoadFilePopup	= false;
		
		App->editor->LoadFileThroughEditor(selectedFile);
		selectedFile[0] = '\0';
	}

	return ret;
}

bool E_LoadFile::CleanUp()
{
	bool ret = true;

	RELEASE_ARRAY(selectedFile);

	return ret;
}

// --- E_LOADFILE METHODS ---
void E_LoadFile::DrawFileBrowser()
{
	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
	ImGui::BeginChild("Asset Files", ImVec2(0, 300), true);

	if (ImGui::TreeNodeEx(ASSETS_PATH, ImGuiTreeNodeFlags_DefaultOpen))
	{
		DrawDirectoriesTree(ASSETS_PATH, DOTLESS_META_EXTENSION);
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
		App->editor->showLoadFilePopup = false;
	}
}

void E_LoadFile::DrawDirectoriesTree(const char* root_directory, const char* extension_to_filter)
{
	std::vector<std::string> directories;
	std::vector<std::string> files;

	std::string root_dir = root_directory;

	App->file_system->DiscoverFiles(root_dir.c_str(), files, directories, extension_to_filter);

	for (uint i = 0; i < directories.size(); ++i)
	{
		std::string path = root_dir + directories[i] + "/";														// Ex: root_dir + directories[i] = "Assets/Models/"
		
		if (ImGui::TreeNodeEx(path.c_str(), 0, "%s/", directories[i].c_str()))
		{
			DrawDirectoriesTree(path.c_str(), extension_to_filter);
			ImGui::TreePop();
		}
	}

	std::sort(files.begin(), files.end());

	for (uint i = 0; i < files.size(); ++i)
	{	
		if (ImGui::TreeNodeEx(files[i].c_str(), ImGuiTreeNodeFlags_Leaf))
		{
			if (ImGui::IsItemClicked())
			{
				sprintf_s(selectedFile, MAX_FILE_SIZE, "%s%s", root_dir.c_str(), files[i].c_str());

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

	directories.clear();
	files.clear();
}