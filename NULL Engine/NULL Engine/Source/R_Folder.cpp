#include "JSONParser.h"

#include "VariableTypedefs.h"

#include "R_Folder.h"

#include "MemoryManager.h"

R_Folder::R_Folder() : Resource(ResourceType::FOLDER)
{

}

R_Folder::~R_Folder()
{

}

bool R_Folder::CleanUp()
{
	bool ret = true;

	containedResources.clear();

	return ret;
}

bool R_Folder::SaveMeta(ParsonNode& metaRoot) const
{
	bool ret = true;

	ParsonArray containedArray = metaRoot.SetArray("ContainedResources");

	return ret;
}

bool R_Folder::LoadMeta(const ParsonNode& metaRoot)
{
	bool ret = true;



	return ret;
}

// --- R_FOLDER METHODS ---
std::vector<uint32> R_Folder::GetContainedResources() const
{
	return containedResources;
}

bool R_Folder::IsContained(uint32 UID) const
{
	for (uint i = 0; i < containedResources.size(); ++i)
	{
		if (containedResources[i] == UID)
		{
			return true;
		}
	}

	return false;
}