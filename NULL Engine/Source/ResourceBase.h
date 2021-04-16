#ifndef __RESOURCE_BASE_H__
#define __RESOURCE_BASE_H__

#include <string>
#include "Macros.h"

class Resource;

enum class ResourceType;

typedef unsigned __int32 uint32;

class NULL_API ResourceBase														// Later make Resource inherit from this.
{
public:
	ResourceBase(uint32 UID, const std::string& assetsPath, const std::string& libraryPath, const ResourceType& type);
	ResourceBase(const Resource* resource);
	~ResourceBase();

	bool CleanUp();

public:
	uint32 UID;

	std::string assetsPath;
	std::string libraryPath;
	
	ResourceType type;

private:

};

#endif // !__RESOURCE_BASE_H__