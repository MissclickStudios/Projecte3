#include "JSONParser.h"

#include "FileSystemDefinitions.h"
#include "VariableTypedefs.h"

#include "Application.h"
#include "M_FileSystem.h"

#include "R_Folder.h"

#include "I_Folders.h"

#include "MemoryManager.h"

uint Importer::Folders::Save(const R_Folder* r_folder, char** buffer)
{
	uint written = 0;
	
	if (r_folder == nullptr)
	{
		LOG("[ERROR] Importer: Could not Save Folder in Library! Error: R_Folder* was nullptr.");
		return 0;
	}
	
	std::string error_string = "[ERROR] Importer: Could not Save Folder { " + std::string(r_folder->GetAssetsPath()) + " } in Library";

	ParsonNode root_node			= ParsonNode();
	ParsonArray contained_array		= root_node.SetArray("ContainedResources");

	for (uint i = 0; i < r_folder->contained_resources.size(); ++i)
	{
		contained_array.SetNumber(r_folder->contained_resources[i]);
	}

	std::string path	= FOLDERS_PATH + std::to_string(r_folder->GetUID()) + FOLDERS_EXTENSION;
	written				= root_node.SerializeToFile(path.c_str(), buffer);
	if (written > 0)
	{
		LOG("[STATUS] Importer: Successfully saved Folder { %s } to Library! Path: %s", r_folder->GetAssetsPath(), path.c_str());
	}
	else
	{
		LOG("%s! Error: File System could not Write File!", error_string.c_str());
	}

	return written;
}

bool Importer::Folders::Load(const char* buffer, R_Folder* r_folder)
{
	bool ret = true;
	
	if (r_folder == nullptr)
	{
		LOG("[ERROR] Importer: Could not Load Folder from Library! Error: R_Folder* was nullptr.");
		return false;
	}
	
	std::string error_string = "[ERROR] Importer: Could not Load Folder { " + std::string(r_folder->GetAssetsPath()) + " } from Library!";

	if (buffer == nullptr)
	{
		LOG("%s! Error: Given buffer was nullptr.", error_string.c_str());
		return false;
	}
	
	ParsonNode root_node			= ParsonNode(buffer);
	ParsonArray contained_array		= root_node.GetArray("ContainedResources");
	if (!root_node.NodeIsValid())
	{
		LOG("%s! Error: Could not get the Root Node from the passed buffer.", error_string.c_str());
		return false;
	}
	if (!contained_array.ArrayIsValid())
	{
		LOG("%s! Error: Could not get the ModelNodes array from the Root Node.", error_string.c_str());
		return false;
	}

	for (uint i = 0; i < contained_array.size; ++i)
	{
		uint32 contained_uid = (uint32)contained_array.GetNumber(i);
		r_folder->contained_resources.push_back(contained_uid);
	}

	LOG("[STATUS] Importer: Successfully Loaded Folder { %s } from Library! UID: %lu", r_folder->GetAssetsPath(), r_folder->GetUID());

	return ret;
}