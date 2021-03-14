#include "R_Script.h"
#include "Application.h"
#include "M_FileSystem.h"

ResourceScript::ResourceScript(): Resource(ResourceType::SCRIPT)
{
}

ResourceScript::~ResourceScript()
{
}

bool ResourceScript::NeedReload() const
{
	return lastTimeMod != App->fileSystem->GetLastModTime(GetAssetsPath());
}
