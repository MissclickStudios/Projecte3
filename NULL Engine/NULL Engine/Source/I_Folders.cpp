#include "JSONParser.h"

#include "FileSystemDefinitions.h"
#include "VariableTypedefs.h"

#include "Application.h"
#include "M_FileSystem.h"

#include "R_Folder.h"

#include "I_Folders.h"

#include "MemoryManager.h"

uint Importer::Folders::Save(const R_Folder* rFolder, char** buffer)
{
	uint written = 0;
	
	if (rFolder == nullptr)
	{
		LOG("[ERROR] Importer: Could not Save Folder in Library! Error: R_Folder* was nullptr.");
		return 0;
	}
	
	std::string errorString = "[ERROR] Importer: Could not Save Folder { " + std::string(rFolder->GetAssetsPath()) + " } in Library";

	ParsonNode rootNode			= ParsonNode();
	ParsonArray containedArray		= rootNode.SetArray("ContainedResources");

	for (uint i = 0; i < rFolder->containedResources.size(); ++i)
	{
		containedArray.SetNumber(rFolder->containedResources[i]);
	}

	std::string path	= FOLDERS_PATH + std::to_string(rFolder->GetUID()) + FOLDERS_EXTENSION;
	written				= rootNode.SerializeToFile(path.c_str(), buffer);
	if (written > 0)
	{
		LOG("[STATUS] Importer: Successfully saved Folder { %s } to Library! Path: %s", rFolder->GetAssetsPath(), path.c_str());
	}
	else
	{
		LOG("%s! Error: File System could not Write File!", errorString.c_str());
	}

	return written;
}

bool Importer::Folders::Load(const char* buffer, R_Folder* rFolder)
{
	bool ret = true;
	
	if (rFolder == nullptr)
	{
		LOG("[ERROR] Importer: Could not Load Folder from Library! Error: R_Folder* was nullptr.");
		return false;
	}
	
	std::string errorString = "[ERROR] Importer: Could not Load Folder { " + std::string(rFolder->GetAssetsPath()) + " } from Library!";

	if (buffer == nullptr)
	{
		LOG("%s! Error: Given buffer was nullptr.", errorString.c_str());
		return false;
	}
	
	ParsonNode rootNode			= ParsonNode(buffer);
	ParsonArray containedArray		= rootNode.GetArray("ContainedResources");
	if (!rootNode.NodeIsValid())
	{
		LOG("%s! Error: Could not get the Root Node from the passed buffer.", errorString.c_str());
		return false;
	}
	if (!containedArray.ArrayIsValid())
	{
		LOG("%s! Error: Could not get the ModelNodes array from the Root Node.", errorString.c_str());
		return false;
	}

	for (uint i = 0; i < containedArray.size; ++i)
	{
		uint32 containedUid = (uint32)containedArray.GetNumber(i);
		rFolder->containedResources.push_back(containedUid);
	}

	LOG("[STATUS] Importer: Successfully Loaded Folder { %s } from Library! UID: %lu", rFolder->GetAssetsPath(), rFolder->GetUID());

	return ret;
}