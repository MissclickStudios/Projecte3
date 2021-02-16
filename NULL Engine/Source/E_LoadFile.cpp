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
selected_file	(nullptr),
ready_to_load	(false)
{
	selected_file = new char[MAX_FILE_SIZE];
	selected_file[0] = '\0';
}

E_LoadFile::~E_LoadFile()
{

}

bool E_LoadFile::Draw(ImGuiIO& io)
{
	bool ret = true;
	
	ImGui::OpenPopup("Load File");

	if (ImGui::BeginPopupModal("Load File"), &App->editor->show_load_file_popup, ImGuiWindowFlags_AlwaysAutoResize)
	{
		DrawFileBrowser();

		DrawFileSelector();

		ImGui::EndPopup();
	}

	if (ready_to_load)
	{
		ready_to_load						= false;
		App->editor->show_load_file_popup	= false;
		
		App->editor->LoadFileThroughEditor(selected_file);
		selected_file[0] = '\0';
	}

	return ret;
}

bool E_LoadFile::CleanUp()
{
	bool ret = true;

	RELEASE_ARRAY(selected_file);

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
	if (ImGui::InputText("##file_selector", selected_file, MAX_FILE_SIZE, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
	{
		ready_to_load = true;
	}
	ImGui::PopItemWidth();

	ImGui::SameLine();

	if (ImGui::Button("Confirm"))
	{
		ready_to_load = true;
	}

	ImGui::SameLine();

	if (ImGui::Button("Cancel"))
	{
		selected_file[0] = '\0';

		ready_to_load = false;
		App->editor->show_load_file_popup = false;
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
				sprintf_s(selected_file, MAX_FILE_SIZE, "%s%s", root_dir.c_str(), files[i].c_str());

				if (ImGui::IsMouseDoubleClicked(0))
				{
					ready_to_load = true;
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