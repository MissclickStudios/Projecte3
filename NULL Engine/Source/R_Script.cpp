#include "R_Script.h"
#include "Application.h"
#include "M_FileSystem.h"
#include "JSONParser.h"

R_Script::R_Script(): Resource(ResourceType::SCRIPT)
{
}

R_Script::~R_Script()
{
}

bool R_Script::SaveMeta(ParsonNode& metaRoot) const
{
	//metaRoot.SetNumber("ResourceUID", (double)GetUID());
	/*ParsonArray containedArray = metaRoot.SetArray("ContainedResources");
	containedArray.SetNumber((double)GetUID());*/
	return true;
}

bool R_Script::LoadMeta(const ParsonNode& metaRoot)
{
	return true;
}

bool R_Script::NeedReload() const
{
	return lastTimeMod != App->fileSystem->GetLastModTime(GetAssetsPath());
}

bool R_Script::CleanUp()
{
	return true;
}
