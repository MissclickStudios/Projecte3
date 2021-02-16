#include "JSONParser.h"

#include "VariableTypedefs.h"

#include "R_Folder.h"

R_Folder::R_Folder() : Resource(RESOURCE_TYPE::FOLDER)
{

}

R_Folder::~R_Folder()
{

}

bool R_Folder::CleanUp()
{
	bool ret = true;

	contained_resources.clear();

	return ret;
}

bool R_Folder::SaveMeta(ParsonNode& meta_root) const
{
	bool ret = true;

	ParsonArray contained_array = meta_root.SetArray("ContainedResources");



	return ret;
}

bool R_Folder::LoadMeta(const ParsonNode& meta_root)
{
	bool ret = true;



	return ret;
}

// --- R_FOLDER METHODS ---
std::vector<uint32> R_Folder::GetContainedResources() const
{
	return contained_resources;
}

bool R_Folder::IsContained(uint32 UID) const
{
	for (uint i = 0; i < contained_resources.size(); ++i)
	{
		if (contained_resources[i] == UID)
		{
			return true;
		}
	}

	return false;
}