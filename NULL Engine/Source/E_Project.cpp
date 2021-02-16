#include <vector>
#include <algorithm>

#include "VariableTypedefs.h"
#include "Macros.h"

#include "PathNode.h"

#include "Application.h"
#include "M_Window.h"
#include "M_FileSystem.h"
#include "M_ResourceManager.h"
#include "M_Editor.h"

#include "Resource.h"
#include "R_Texture.h"

#include "E_Project.h"

#include "MemoryManager.h"

#define MAX_DIRECTORY_SIZE	500

E_Project::E_Project() : EditorPanel("Project"),
directory_to_display			(nullptr),
refresh_root_directory			(true),
refresh_directory_to_display	(false),
refresh_window_size				(true),
icons_are_loaded				(false),
dragged_resource				(nullptr)
{
	directory_to_display = new char[MAX_DIRECTORY_SIZE];
	sprintf_s(directory_to_display, MAX_DIRECTORY_SIZE, "%s", ASSETS_PATH);

	icon_size		= ImVec2(64.0f, 64.0f);
	icon_offset		= ImVec2(20.0f, 0.0f);
	text_offset		= ImVec2(icon_offset.x, icon_size.y);
	win_size		= ImVec2(0.0f, 0.0f);
}

E_Project::~E_Project()
{
	delete[] directory_to_display;
}

bool E_Project::Draw(ImGuiIO& io)
{
	bool ret = true;

	CheckFlags();

	ImGui::Begin("Project##", (bool*)0, ImGuiWindowFlags_MenuBar);

	GenerateDockspace(io);
	
	DrawMenuBar();
	DrawAssetsTree();
	DrawFolderExplorer();

	ImGui::End();

	return ret;
}

bool E_Project::CleanUp()
{
	bool ret = true;

	root_directory.children.clear();
	dragged_resource = nullptr;
	ClearResourcesToDisplay();

	return ret;
}

// --- E_PRROJECT METHODS ---
Resource* E_Project::GetDraggedResource()
{
	return dragged_resource;
}

void E_Project::CheckFlags()
{
	if (!icons_are_loaded)
	{
		App->editor->GetEngineIconsThroughEditor(engine_icons);
		icons_are_loaded = true;
	}
	
	if (refresh_window_size)
	{
		OnResize();
		refresh_window_size = false;
	}

	if (refresh_root_directory)
	{
		std::vector<std::string> extensions_to_filter;
		extensions_to_filter.push_back("meta");

		root_directory = App->file_system->GetAllFiles(ASSETS_DIRECTORY, nullptr, &extensions_to_filter);

		extensions_to_filter.clear();

		refresh_root_directory = false;
	}

	if (refresh_directory_to_display)
	{
		if (directory_to_display == nullptr)
		{
			refresh_directory_to_display = false;
			return;
		}
		
		ClearResourcesToDisplay();

		bool success = root_directory.FindChild(directory_to_display, display_directory);
		if (!success)
		{
			LOG("[ERROR] Editor Project Panel: Could not Refresh Directory to Display! Error: Could not get { %s }'s PathNode.", directory_to_display);
			refresh_directory_to_display = false;
			return;
		}

		for (uint i = 0; i < display_directory.children.size(); ++i)
		{
			Resource* resource = App->resource_manager->GetResourceFromMetaFile(display_directory.children[i].path.c_str());

			if (resource != nullptr)
			{
				resources_to_display.push_back(resource);
			}
		}

		refresh_directory_to_display = false;
	}
}

void E_Project::OnResize()
{
	win_size = ImVec2((float)App->window->GetWidth(), (float)App->window->GetHeight());
}

void E_Project::GenerateDockspace(ImGuiIO& io) const
{
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		ImGuiID dockspace_id = ImGui::GetID("Project##");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
	}
}

void E_Project::DrawMenuBar() const
{
	ImGui::BeginMenuBar();

	if (ImGui::BeginMenu("Options"))
	{
		ImGui::MenuItem("Show In Explorer", nullptr, false, false);

		ImGui::EndMenu();
	}

	ImGui::EndMenuBar();
}

void E_Project::DrawAssetsTree()
{
	ImGui::Begin("AssetsTree", false);

	if (ImGui::TreeNodeEx(ASSETS_PATH, ImGuiTreeNodeFlags_DefaultOpen))
	{
		//DrawDirectoriesTree(ASSETS_PATH, DOTLESS_META_EXTENSION);
		DrawDirectoriesTree(root_directory);
		ImGui::TreePop();
	}

	ImGui::End();
}

void E_Project::DrawFolderExplorer()
{
	ImGui::Begin("FolderExplorer", false);

	ImGui::Text(directory_to_display);

	ImGui::Separator();

	DrawResourceIcons();

	ImGui::End();
}

void E_Project::DrawDirectoriesTree(const char* root_directory, const char* extension_to_filter)
{
	ImGuiTreeNodeFlags tree_node_flags = ImGuiTreeNodeFlags_None;

	std::vector<std::string> directories;
	std::vector<std::string> files;
	std::string root_dir = root_directory;
	
	App->file_system->DiscoverFiles(root_dir.c_str(), files, directories, extension_to_filter);

	for (uint i = 0; i < directories.size(); ++i)
	{
		std::string path	= root_dir + directories[i] + ("/");
		tree_node_flags		= (!App->file_system->ContainsDirectory(path.c_str())) ? ImGuiTreeNodeFlags_Leaf : ImGuiTreeNodeFlags_None;

		if (ImGui::TreeNodeEx(path.c_str(), tree_node_flags, "%s/", directories[i].c_str()))
		{
			if (ImGui::IsItemClicked())
			{
				sprintf_s(directory_to_display, MAX_DIRECTORY_SIZE, "%s%s/", root_dir.c_str(), directories[i].c_str());
				refresh_directory_to_display = true;
			}
			
			DrawDirectoriesTree(path.c_str(), extension_to_filter);

			ImGui::TreePop();
		}
	}

	directories.clear();
	files.clear();
}

void E_Project::DrawDirectoriesTree(const PathNode& root_node)
{
	ImGuiTreeNodeFlags tree_node_flags	= ImGuiTreeNodeFlags_None;
	std::string path					= "[NONE]";
	std::string directory				= "[NONE]";

	for (uint i = 0; i < root_node.children.size(); ++i)
	{
		PathNode path_node = root_node.children[i];

		if (/*path_node.is_file*/ !App->file_system->IsDirectory(path_node.path.c_str()))
		{
			continue;
		}

		path			= path_node.path;
		directory		= path_node.local_path;
		tree_node_flags = (path_node.is_last_directory) ? ImGuiTreeNodeFlags_Leaf : ImGuiTreeNodeFlags_None;
		if (ImGui::TreeNodeEx(path.c_str(), tree_node_flags, "%s/", directory.c_str()))
		{
			if (ImGui::IsItemClicked())
			{
				if (strcmp(directory_to_display, path.c_str()) != 0)
				{
					sprintf_s(directory_to_display, MAX_DIRECTORY_SIZE, "%s", path.c_str());
					refresh_directory_to_display = true;
				}
			}

			if (!path_node.is_last_directory)
			{
				DrawDirectoriesTree(path_node);
			}

			ImGui::TreePop();
		}
	}
}

void E_Project::DrawResourceIcons()
{
	ImVec2 uv_0			= ImVec2(0.0f, 1.0f);
	ImVec2 uv_1			= ImVec2(1.0f, 0.0f);
	ImVec2 padding		= ImVec2(0.0f, 0.0f);
	ImVec4 bg_color		= ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	ImVec4 tint_color	= ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

	ImVec2 original_pos		= ImVec2(0.0f, 0.0f);
	ImTextureID tex_id		= 0;
	ImVec2 item_offset		= ImVec2(icon_size.x + icon_offset.x, 0.0f);
	ImVec2 next_item_pos	= ImVec2(0.0f, 0.0f);

	GoToPreviousDirectoryButton();

	for (uint i = 0; i < resources_to_display.size(); ++i)
	{	
		original_pos = ImGui::GetCursorPos();
		
		tex_id = GetIconTexID(resources_to_display[i]);
		ImGui::SetCursorPos(original_pos + icon_offset);
		//ImGui::ImageButtonEx(i + 1, tex_id, icon_size, uv_0, uv_1, padding, bg_color, tint_color);
		ImGui::Image(tex_id, icon_size, uv_0, uv_1, tint_color, bg_color);

		if (resources_to_display[i]->GetType() == RESOURCE_TYPE::FOLDER)
		{
			if (ImGui::IsItemClicked())
			{
				std::string path = resources_to_display[i]->GetAssetsPath();
				if (strcmp(directory_to_display , path.c_str()) != 0)
				{
					sprintf_s(directory_to_display, MAX_DIRECTORY_SIZE, "%s", path.c_str());
					refresh_directory_to_display = true;
					return;
				}
			}
		}
		else
		{
			ResourceDragAndDropEvent(resources_to_display[i], tex_id);
		}

		ImGui::SetCursorPos(original_pos + text_offset);
		ImGui::Text(GetDisplayString(resources_to_display[i]->GetAssetsFile(), 8).c_str());

		next_item_pos = original_pos + item_offset;
		if (next_item_pos.x + item_offset.x < ImGui::GetWindowWidth())
		{
			ImGui::SetCursorPos(next_item_pos);
		}
	}
}

void E_Project::GoToPreviousDirectoryButton()
{
	ImVec2 uv_0			= ImVec2(0.0f, 1.0f);
	ImVec2 uv_1			= ImVec2(1.0f, 0.0f);
	ImVec4 bg_color		= ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	ImVec4 tint_color	= ImVec4(1.0f, 1.0f, 1.0f, 0.5f);
	
	ImVec2 original_pos = ImGui::GetCursorPos();
	ImVec2 item_offset	= ImVec2(icon_size.x + icon_offset.x, 0.0f);
	
	ImGui::SetCursorPos(original_pos + icon_offset);
	ImGui::Image((ImTextureID)engine_icons.folder_icon->GetTextureID(), icon_size, uv_0, uv_1, tint_color, bg_color);

	if (ImGui::IsItemClicked())
	{
		std::string prev_dir	= directory_to_display;
		uint end_pos			= prev_dir.find_last_of("/");
		prev_dir				= prev_dir.substr(0, end_pos);

		sprintf_s(directory_to_display, MAX_DIRECTORY_SIZE, "%s", prev_dir.c_str());
		refresh_directory_to_display = true;
	}

	ImGui::SetCursorPos(original_pos + text_offset);
	ImGui::Text("../");

	ImVec2 next_item_pos = original_pos + item_offset;
	if (next_item_pos.x + item_offset.x < ImGui::GetWindowWidth())
	{
		ImGui::SetCursorPos(next_item_pos);
	}
}

void E_Project::ResourceDragAndDropEvent(Resource* resource, ImTextureID texture_id)
{
	if (resource == nullptr)
	{
		LOG("[ERROR] Editor Project Panel: Could not check for Resource Drag&Drop Event! Error: Given Resource* was nullptr.");
		return;
	}

	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
	{	
		ImGui::SetDragDropPayload("DRAGGED_RESOURCE", resource, sizeof(Resource));
	
		ImGui::Text("Dragging %s", resource->GetAssetsFile());
		ImGui::Image(texture_id, icon_size);

		dragged_resource = resource;

		ImGui::EndDragDropSource();
	}

	/*if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DRAGGED_RESOURCE"))
		{
			LOG("[WARNING] MAYBE PUT THIS AS A LISTENER IN VIEWPORT???");
		}

		ImGui::EndDragDropTarget();
	}*/
}

ImTextureID E_Project::GetIconTexID(Resource* resource) const
{
	ImTextureID tex_id = 0;

	if (resource == nullptr)
	{
		LOG("[ERROR] Editor Project Panel: Could not get Icon Texture ID! Error: Given Resource* was nullptr.");
		return 0;
	}

	RESOURCE_TYPE type = resource->GetType();
	switch (type)
	{
	case RESOURCE_TYPE::MODEL:		{ tex_id = (ImTextureID)engine_icons.model_icon->GetTextureID(); }		break;
	case RESOURCE_TYPE::MESH:		{ tex_id = (ImTextureID)engine_icons.file_icon->GetTextureID(); }		break;
	case RESOURCE_TYPE::MATERIAL:	{ tex_id = (ImTextureID)engine_icons.material_icon->GetTextureID(); }	break;
	case RESOURCE_TYPE::TEXTURE:	{ tex_id = (ImTextureID)(((R_Texture*)resource)->GetTextureID()); }		break;
	case RESOURCE_TYPE::FOLDER:		{ tex_id = (ImTextureID)engine_icons.folder_icon->GetTextureID(); }		break;
	case RESOURCE_TYPE::SCENE:		{ tex_id = (ImTextureID)engine_icons.model_icon->GetTextureID(); }		break;
	case RESOURCE_TYPE::ANIMATION:	{ tex_id = (ImTextureID)engine_icons.animation_icon->GetTextureID(); }	break;
	}

	return tex_id;
}

std::string E_Project::GetDisplayString(std::string original_string, uint max_length) const
{	
	if (original_string.length() <= max_length)
	{
		return original_string;
	}
	if (max_length == 0)
	{
		return std::string("");
	}

	std::string display_string = original_string;

	display_string.resize(max_length);
	display_string.append("...");

	return display_string;
}

void E_Project::ClearResourcesToDisplay()
{
	for (uint i = 0; i < resources_to_display.size(); ++i)
	{
		App->resource_manager->FreeResource(resources_to_display[i]->GetUID());
	}
	
	resources_to_display.clear();
}