#include <vector>
#include <algorithm>
#include <utility>

#include "Profiler.h"
//#include "Prefab.h"

#include "VariableTypedefs.h"
#include "Macros.h"

#include "PathNode.h"

#include "EngineApplication.h"
#include "FileSystemDefinitions.h"
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
refreshRootDirectory		(true),
refreshDirectoryToDisplay	(false),
refreshWindowSize			(true),
iconsAreLoaded				(false)
{
	directoryToDisplay = new char[MAX_DIRECTORY_SIZE];
	sprintf_s(directoryToDisplay, MAX_DIRECTORY_SIZE, "%s", ASSETS_PATH);

	iconSize		= ImVec2(64.0f, 64.0f);
	iconOffset		= ImVec2(20.0f, 0.0f);
	textOffset		= ImVec2(iconOffset.x, iconSize.y);
	winSize			= ImVec2(0.0f, 0.0f);
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
	ClearAssetsToDisplay();

	return ret;
}

// --- E_PROJECT METHODS ---
void E_Project::CheckFlags()
{
	if (!iconsAreLoaded)
	{
		EngineApp->editor->GetEngineIconsThroughEditor(engineIcons);
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

		rootDirectory = EngineApp->fileSystem->GetAllFiles(ASSETS_DIRECTORY, nullptr, &extensionsToFilter);

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
		
		ClearAssetsToDisplay();

		bool success = rootDirectory.FindChild(directoryToDisplay, displayDirectory);
		if (!success)
		{
			LOG("[ERROR] Editor Project Panel: Could not Refresh Directory to Display! Error: Could not get { %s }'s PathNode.", directoryToDisplay);
			refreshDirectoryToDisplay = false;
			return;
		}

		for (uint i = 0; i < displayDirectory.children.size(); ++i)
		{
			const char* path		= displayDirectory.children[i].path.c_str();
			std::string file		= EngineApp->fileSystem->GetFileAndExtension(path);
			ResourceType type		= EngineApp->resourceManager->GetTypeFromAssetsExtension(path);
			
			R_Texture* assetTexture = nullptr;
			if (type == ResourceType::TEXTURE)
			{
				assetTexture = (R_Texture*)EngineApp->resourceManager->GetResourceFromLibrary(path);
			}
			
			if (type != ResourceType::NONE) //TODO check this
			{
				assetsToDisplay.push_back({ path, file, type, assetTexture });
			}
		}

		refreshDirectoryToDisplay = false;
	}
}

void E_Project::OnResize()
{
	winSize = ImVec2((float)EngineApp->window->GetWidth(), (float)EngineApp->window->GetHeight());
}

void E_Project::GenerateDockspace(ImGuiIO& io) const
{
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		ImGuiID dockspaceId = ImGui::GetID("Project##");
		ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
	}
}

void E_Project::DrawMenuBar()
{
	ImGui::BeginMenuBar();

	if (ImGui::BeginMenu("Options"))
	{
		ImGui::MenuItem("Show In Explorer", nullptr, false, false);

		if (ImGui::MenuItem("Refresh All Directories", nullptr, false))
		{
			App->resourceManager->RefreshProjectDirectories();
			refreshRootDirectory		= true;
			refreshDirectoryToDisplay	= true;
		}

		if (ImGui::MenuItem("Refresh Current Directory", nullptr, false))
		{
			App->resourceManager->RefreshProjectDirectory(directoryToDisplay);
			refreshRootDirectory		= true;
			refreshDirectoryToDisplay	= true;
		}

		ImGui::EndMenu();
	}

	ImGui::EndMenuBar();
}

void E_Project::DrawAssetsTree()
{
	bool retraso = false;
	ImGui::Begin("AssetsTree", &retraso);

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
	ImGui::Begin("FolderExplorer", (bool*)0);

	ImGui::Text(directoryToDisplay);
	ImGui::SameLine(ImGui::GetWindowWidth() * 0.3f);
	if (ImGui::Button("Refresh Current Directory"))
	{ 
		App->resourceManager->RefreshProjectDirectory(directoryToDisplay);
		refreshRootDirectory		= true;
		refreshDirectoryToDisplay	= true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Refresh All Directories"))
	{ 
		App->resourceManager->RefreshProjectDirectories(); 
		refreshRootDirectory		= true;
		refreshDirectoryToDisplay	= true;
	}

	ImGui::Separator();

	DrawResourceIcons();

	ImGui::End();
}

void E_Project::DrawDirectoriesTree(const char* rootDirectory, const char* extensionToFilter)
{
	ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_None;

	std::vector<std::string> directories;
	std::vector<std::string> files;
	std::string rootDir = rootDirectory;
	
	EngineApp->fileSystem->DiscoverFiles(rootDir.c_str(), files, directories, extensionToFilter);

	for (uint i = 0; i < directories.size(); ++i)
	{
		std::string path	= rootDir + directories[i] + ("/");
		treeNodeFlags		= (!EngineApp->fileSystem->ContainsDirectory(path.c_str())) ? ImGuiTreeNodeFlags_Leaf : ImGuiTreeNodeFlags_None;

		if (ImGui::TreeNodeEx(path.c_str(), treeNodeFlags, "%s/", directories[i].c_str()))
		{
			if (ImGui::IsItemClicked())
			{
				sprintf_s(directoryToDisplay, MAX_DIRECTORY_SIZE, "%s%s/", rootDir.c_str(), directories[i].c_str());
				refreshDirectoryToDisplay = true;
			}
			
			DrawDirectoriesTree(path.c_str(), extensionToFilter);

			ImGui::TreePop();
		}
	}

	directories.clear();
	files.clear();
}

void E_Project::DrawDirectoriesTree(const PathNode& rootNode)
{
	ImGuiTreeNodeFlags treeNodeFlags	= ImGuiTreeNodeFlags_None;
	std::string path					= "[NONE]";
	std::string directory				= "[NONE]";

	for (uint i = 0; i < rootNode.children.size(); ++i)
	{
		PathNode pathNode = rootNode.children[i];

		if (/*path_node.is_file*/ !EngineApp->fileSystem->IsDirectory(pathNode.path.c_str()))
		{
			continue;
		}

		path			= pathNode.path;
		directory		= pathNode.local_path;
		treeNodeFlags	= (pathNode.isLastDirectory) ? ImGuiTreeNodeFlags_Leaf : ImGuiTreeNodeFlags_None;
		if (ImGui::TreeNodeEx(path.c_str(), treeNodeFlags, "%s", directory.c_str()))
		{
			if (ImGui::IsItemClicked())
			{
				if (strcmp(directoryToDisplay, path.c_str()) != 0)
				{
					sprintf_s(directoryToDisplay, MAX_DIRECTORY_SIZE, "%s", path.c_str());
					refreshDirectoryToDisplay = true;
				}
			}

			if (!pathNode.isLastDirectory)
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

	ImVec2 originalPos	= ImVec2(0.0f, 0.0f);
	ImTextureID texID	= 0;
	ImVec2 itemOffset	= ImVec2(iconSize.x + iconOffset.x, 0.0f);
	ImVec2 nextItemPos	= ImVec2(0.0f, 0.0f);

	DrawGoToPreviousDirectoryButton();

	for (auto item = assetsToDisplay.begin(); item != assetsToDisplay.end(); ++item)
	{
		originalPos = ImGui::GetCursorPos();
		ImGui::SetCursorPos(originalPos + iconOffset);

		texID = GetIconTexID((*item));

		if (texID == 0)
		{
			continue;
		}
		else
		{
			ImGui::Image(texID, iconSize, uv0, uv1, tintColor, bgColor);
		}

		if (item->type == ResourceType::FOLDER)
		{
			if (ImGui::IsItemClicked())
			{
				std::string path = item->path;
				if (strcmp(directoryToDisplay, path.c_str()) != 0)
				{
					sprintf_s(directoryToDisplay, MAX_DIRECTORY_SIZE, "%s", path.c_str());
					refreshDirectoryToDisplay = true;
					return;
				}
			}
		}
		else
		{
			AssetDragAndDropEvent(item->path, texID);
		}

		ImGui::SetCursorPos(originalPos + textOffset);

		if (item->type == ResourceType::PREFAB)
		{
			std::string prefabName = "Prefab";
			std::map<uint,Prefab>::iterator a = EngineApp->resourceManager->prefabs.find(atoi(item->file.c_str()));
			if (a != EngineApp->resourceManager->prefabs.end())
				prefabName = a->second.name;

			ImGui::Text(GetDisplayString(prefabName, 8).c_str());
		}
		else
			ImGui::Text(GetDisplayString(item->file, 8).c_str());

		nextItemPos = originalPos + itemOffset;
		if (nextItemPos.x + itemOffset.x < ImGui::GetWindowWidth())
		{
			ImGui::SetCursorPos(nextItemPos);
		}
	}
}

void E_Project::DrawGoToPreviousDirectoryButton()
{
	ImVec2 uv0			= ImVec2(0.0f, 1.0f);
	ImVec2 uv1			= ImVec2(1.0f, 0.0f);
	ImVec4 bgColor		= ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	ImVec4 tintColor	= ImVec4(1.0f, 1.0f, 1.0f, 0.5f);
	
	ImVec2 originalPos	= ImGui::GetCursorPos();
	ImVec2 itemOffset	= ImVec2(iconSize.x + iconOffset.x, 0.0f);
	
	ImGui::SetCursorPos(originalPos + iconOffset);
	ImGui::Image((ImTextureID)engineIcons.folderIcon->GetTextureID(), iconSize, uv0, uv1, tintColor, bgColor);

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

void E_Project::AssetDragAndDropEvent(const char* assetPath, ImTextureID textureID)
{
	if (assetPath == nullptr)
	{
		LOG("[ERROR] Editor Project Panel: Could not check for Resource Drag&Drop Event! Error: Given assetsPath* was nullptr.");
		return;
	}

	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
	{
		ImGui::SetDragDropPayload("DRAGGED_ASSET", &assetPath, sizeof(const char*),ImGuiCond_Once);

		ImGui::Text("Dragging %s", assetPath);
		ImGui::Image(textureID, iconSize);

		ImGui::EndDragDropSource();
	}
}

ImTextureID E_Project::GetIconTexID(const AssetDisplay& assetDisplay) const
{
	ImTextureID texId = 0;

	if (assetDisplay.type == ResourceType::NONE)
	{
		LOG("[ERROR] Editor Project Panel: Could not get Icon Texture ID! Error: Given Resource* was nullptr.");
		return 0;
	}

	switch (assetDisplay.type)
	{
	case ResourceType::MODEL:		{ texId = (ImTextureID)engineIcons.modelIcon->GetTextureID(); }			break;
	case ResourceType::MESH:		{ texId = (ImTextureID)engineIcons.fileIcon->GetTextureID(); }			break;
	case ResourceType::MATERIAL:	{ texId = (ImTextureID)engineIcons.materialIcon->GetTextureID(); }		break;
	case ResourceType::TEXTURE:		{ texId = (ImTextureID)GetOGLTextureID(assetDisplay.assetTexture); }	break;
	case ResourceType::FOLDER:		{ texId = (ImTextureID)engineIcons.folderIcon->GetTextureID(); }		break;
	case ResourceType::SCENE:		{ texId = (ImTextureID)engineIcons.modelIcon->GetTextureID(); }			break;
	case ResourceType::ANIMATION:	{ texId = (ImTextureID)engineIcons.animationIcon->GetTextureID(); }		break;
	case ResourceType::PREFAB:		{ texId = (ImTextureID)engineIcons.modelIcon->GetTextureID(); }			break;
	}

	return texId;
}

uint E_Project::GetOGLTextureID(R_Texture* assetTexture) const
{	
	if (assetTexture == nullptr)
	{
		LOG("[ERROR] Editor Project Panel: Could not get OGL Texture ID! Error: Given R_Texture* was nullptr.");
		return 0;
	}

	return assetTexture->GetTextureID();
}

std::string E_Project::GetDisplayString(std::string originalString, uint maxLength) const
{	
	if (originalString.length() <= maxLength)
	{
		return originalString;
	}
	if (maxLength == 0)
	{
		return std::string("");
	}

	std::string displayString = originalString;

	displayString.resize(maxLength);
	displayString.append("...");

	return displayString;
}

void E_Project::ClearAssetsToDisplay()
{
	for (auto item = assetsToDisplay.begin(); item != assetsToDisplay.end(); ++item)
	{
		if (item->type == ResourceType::TEXTURE && item->assetTexture != nullptr) //TODO check this
		{
			EngineApp->resourceManager->FreeResource(item->assetTexture->GetUID());
		}

		//EngineApp->resourceManager->FreeResource(resourcesToDisplay[i]->GetUID());
	}

	assetsToDisplay.clear();
}