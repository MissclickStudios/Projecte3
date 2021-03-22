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
	//If not meta is not valid
	ParsonArray contained_array = metaRoot.SetArray("ContainedResources");

	ParsonNode settings = metaRoot.SetNode("ImportSettings");
	//meshSettings.Save(settings); //TODO: Script Import settings

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
