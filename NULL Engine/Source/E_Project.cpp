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
directoryToDisplay			(nullptr),
refreshRootDirectory			(true),
refreshDirectoryToDisplay	(false),
refreshWindowSize				(true),
iconsAreLoaded				(false),
draggedResource				(nullptr)
{
	directoryToDisplay = new char[MAX_DIRECTORY_SIZE];
	sprintf_s(directoryToDisplay, MAX_DIRECTORY_SIZE, "%s", ASSETS_PATH);

	iconSize		= ImVec2(64.0f, 64.0f);
	iconOffset		= ImVec2(20.0f, 0.0f);
	textOffset		= ImVec2(iconOffset.x, iconSize.y);
	winSize		= ImVec2(0.0f, 0.0f);
}

E_Project::~E_Project()
{
	delete[] directoryToDisplay;
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

	rootDirectory.children.clear();
	draggedResource = nullptr;
	ClearResourcesToDisplay();

	return ret;
}

// --- E_PRROJECT METHODS ---
Resource* E_Project::GetDraggedResource()
{
	return draggedResource;
}

void E_Project::CheckFlags()
{
	if (!iconsAreLoaded)
	{
		App->editor->GetEngineIconsThroughEditor(engineIcons);
		iconsAreLoaded = true;
	}
	
	if (refreshWindowSize)
	{
		OnResize();
		refreshWindowSize = false;
	}

	if (refreshRootDirectory)
	{
		std::vector<std::string> extensionsToFilter;
		extensionsToFilter.push_back("meta");

		rootDirectory = App->fileSystem->GetAllFiles(ASSETS_DIRECTORY, nullptr, &extensionsToFilter);

		extensionsToFilter.clear();

		refreshRootDirectory = false;
	}

	if (refreshDirectoryToDisplay)
	{
		if (directoryToDisplay == nullptr)
		{
			refreshDirectoryToDisplay = false;
			return;
		}
		
		ClearResourcesToDisplay();

		bool success = rootDirectory.FindChild(directoryToDisplay, displayDirectory);
		if (!success)
		{
			LOG("[ERROR] Editor Project Panel: Could not Refresh Directory to Display! Error: Could not get { %s }'s PathNode.", directoryToDisplay);
			refreshDirectoryToDisplay = false;
			return;
		}

		for (uint i = 0; i < displayDirectory.children.size(); ++i)
		{
			Resource* resource = App->resourceManager->GetResourceFromMetaFile(displayDirectory.children[i].path.c_str());

			if (resource != nullptr)
			{
				resourcesToDisplay.push_back(resource);
			}
		}

		refreshDirectoryToDisplay = false;
	}
}

void E_Project::OnResize()
{
	winSize = ImVec2((float)App->window->GetWidth(), (float)App->window->GetHeight());
}

void E_Project::GenerateDockspace(ImGuiIO& io) const
{
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		ImGuiID dockspaceId = ImGui::GetID("Project##");
		ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
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
		DrawDirectoriesTree(rootDirectory);
		ImGui::TreePop();
	}

	ImGui::End();
}

void E_Project::DrawFolderExplorer()
{
	ImGui::Begin("FolderExplorer", false);

	ImGui::Text(directoryToDisplay);

	ImGui::Separator();

	DrawResourceIcons();

	ImGui::End();
}

void E_Project::DrawDirectoriesTree(const char* root_directory, const char* extension_to_filter)
{
	ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_None;

	std::vector<std::string> directories;
	std::vector<std::string> files;
	std::string rootDir = root_directory;
	
	App->fileSystem->DiscoverFiles(rootDir.c_str(), files, directories, extension_to_filter);

	for (uint i = 0; i < directories.size(); ++i)
	{
		std::string path	= rootDir + directories[i] + ("/");
		treeNodeFlags		= (!App->fileSystem->ContainsDirectory(path.c_str())) ? ImGuiTreeNodeFlags_Leaf : ImGuiTreeNodeFlags_None;

		if (ImGui::TreeNodeEx(path.c_str(), treeNodeFlags, "%s/", directories[i].c_str()))
		{
			if (ImGui::IsItemClicked())
			{
				sprintf_s(directoryToDisplay, MAX_DIRECTORY_SIZE, "%s%s/", rootDir.c_str(), directories[i].c_str());
				refreshDirectoryToDisplay = true;
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
	ImGuiTreeNodeFlags treeNodeFlags	= ImGuiTreeNodeFlags_None;
	std::string path					= "[NONE]";
	std::string directory				= "[NONE]";

	for (uint i = 0; i < root_node.children.size(); ++i)
	{
		PathNode pathNode = root_node.children[i];

		if (/*path_node.is_file*/ !App->fileSystem->IsDirectory(pathNode.path.c_str()))
		{
			continue;
		}

		path			= pathNode.path;
		directory		= pathNode.local_path;
		treeNodeFlags = (pathNode.is_last_directory) ? ImGuiTreeNodeFlags_Leaf : ImGuiTreeNodeFlags_None;
		if (ImGui::TreeNodeEx(path.c_str(), treeNodeFlags, "%s/", directory.c_str()))
		{
			if (ImGui::IsItemClicked())
			{
				if (strcmp(directoryToDisplay, path.c_str()) != 0)
				{
					sprintf_s(directoryToDisplay, MAX_DIRECTORY_SIZE, "%s", path.c_str());
					refreshDirectoryToDisplay = true;
				}
			}

			if (!pathNode.is_last_directory)
			{
				DrawDirectoriesTree(pathNode);
			}

			ImGui::TreePop();
		}
	}
}

void E_Project::DrawResourceIcons()
{
	ImVec2 uv0			= ImVec2(0.0f, 1.0f);
	ImVec2 uv1			= ImVec2(1.0f, 0.0f);
	ImVec2 padding		= ImVec2(0.0f, 0.0f);
	ImVec4 bgColor		= ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	ImVec4 tintColor	= ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

	ImVec2 originalPos		= ImVec2(0.0f, 0.0f);
	ImTextureID texId		= 0;
	ImVec2 itemOffset		= ImVec2(iconSize.x + iconOffset.x, 0.0f);
	ImVec2 nextItemPos	= ImVec2(0.0f, 0.0f);

	GoToPreviousDirectoryButton();

	for (uint i = 0; i < resourcesToDisplay.size(); ++i)
	{	
		originalPos = ImGui::GetCursorPos();
		
		texId = GetIconTexID(resourcesToDisplay[i]);
		ImGui::SetCursorPos(originalPos + iconOffset);
		//ImGui::ImageButtonEx(i + 1, tex_id, icon_size, uv_0, uv_1, padding, bg_color, tint_color);
		ImGui::Image(texId, iconSize, uv0, uv1, tintColor, bgColor);

		if (resourcesToDisplay[i]->GetType() == RESOURCE_TYPE::FOLDER)
		{
			if (ImGui::IsItemClicked())
			{
				std::string path = resourcesToDisplay[i]->GetAssetsPath();
				if (strcmp(directoryToDisplay , path.c_str()) != 0)
				{
					sprintf_s(directoryToDisplay, MAX_DIRECTORY_SIZE, "%s", path.c_str());
					refreshDirectoryToDisplay = true;
					return;
				}
			}
		}
		else
		{
			ResourceDragAndDropEvent(resourcesToDisplay[i], texId);
		}

		ImGui::SetCursorPos(originalPos + textOffset);
		ImGui::Text(GetDisplayString(resourcesToDisplay[i]->GetAssetsFile(), 8).c_str());

		nextItemPos = originalPos + itemOffset;
		if (nextItemPos.x + itemOffset.x < ImGui::GetWindowWidth())
		{
			ImGui::SetCursorPos(nextItemPos);
		}
	}
}

void E_Project::GoToPreviousDirectoryButton()
{
	ImVec2 uv0			= ImVec2(0.0f, 1.0f);
	ImVec2 uv1			= ImVec2(1.0f, 0.0f);
	ImVec4 bgColor		= ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	ImVec4 tintColor	= ImVec4(1.0f, 1.0f, 1.0f, 0.5f);
	
	ImVec2 originalPos = ImGui::GetCursorPos();
	ImVec2 itemOffset	= ImVec2(iconSize.x + iconOffset.x, 0.0f);
	
	ImGui::SetCursorPos(originalPos + iconOffset);
	ImGui::Image((ImTextureID)engineIcons.folder_icon->GetTextureID(), iconSize, uv0, uv1, tintColor, bgColor);

	if (ImGui::IsItemClicked())
	{
		std::string prevDir	= directoryToDisplay;
		uint endPos			= prevDir.find_last_of("/");
		prevDir				= prevDir.substr(0, endPos);

		sprintf_s(directoryToDisplay, MAX_DIRECTORY_SIZE, "%s", prevDir.c_str());
		refreshDirectoryToDisplay = true;
	}

	ImGui::SetCursorPos(originalPos + textOffset);
	ImGui::Text("../");

	ImVec2 nextItemPos = originalPos + itemOffset;
	if (nextItemPos.x + itemOffset.x < ImGui::GetWindowWidth())
	{
		ImGui::SetCursorPos(nextItemPos);
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
		ImGui::Image(texture_id, iconSize);

		draggedResource = resource;

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
	ImTextureID texId = 0;

	if (resource == nullptr)
	{
		LOG("[ERROR] Editor Project Panel: Could not get Icon Texture ID! Error: Given Resource* was nullptr.");
		return 0;
	}

	RESOURCE_TYPE type = resource->GetType();
	switch (type)
	{
	case RESOURCE_TYPE::MODEL:		{ texId = (ImTextureID)engineIcons.model_icon->GetTextureID(); }		break;
	case RESOURCE_TYPE::MESH:		{ texId = (ImTextureID)engineIcons.file_icon->GetTextureID(); }		break;
	case RESOURCE_TYPE::MATERIAL:	{ texId = (ImTextureID)engineIcons.material_icon->GetTextureID(); }	break;
	case RESOURCE_TYPE::TEXTURE:	{ texId = (ImTextureID)(((R_Texture*)resource)->GetTextureID()); }		break;
	case RESOURCE_TYPE::FOLDER:		{ texId = (ImTextureID)engineIcons.folder_icon->GetTextureID(); }		break;
	case RESOURCE_TYPE::SCENE:		{ texId = (ImTextureID)engineIcons.model_icon->GetTextureID(); }		break;
	case RESOURCE_TYPE::ANIMATION:	{ texId = (ImTextureID)engineIcons.animation_icon->GetTextureID(); }	break;
	}

	return texId;
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

	std::string displayString = original_string;

	displayString.resize(max_length);
	displayString.append("...");

	return displayString;
}

void E_Project::ClearResourcesToDisplay()
{
	for (uint i = 0; i < resourcesToDisplay.size(); ++i)
	{
		App->resourceManager->FreeResource(resourcesToDisplay[i]->GetUID());
	}
	
	resourcesToDisplay.clear();
}