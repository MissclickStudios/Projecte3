#include "R_Script.h"

ResourceScript::ResourceScript(): Resource(ResourceType::SCRIPT)
{
}

ResourceScript::~ResourceScript()
{
}

bool ResourceScript::NeedReload() const
{
	/*bool ret = false;

	struct stat file;
	if (stat(meta_data_path.c_str(), &file) == 0)
	{
		if (file.st_mtime != last_time_mod) {
			ret = true;
		}
	}

	return ret;*/
	return true;
}
