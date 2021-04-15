#ifndef __RESOURCE_BASE_H__
#define __RESOURCE_BASE_H__

#include <string>

enum class ResourceType;

class ResourceBase														// Later make Resource inherit from this.
{
public:
	ResourceBase(std::string libraryPath, ResourceType type);
	~ResourceBase();

	bool CleanUp();

public:
	std::string libraryPath;
	ResourceType type;

private:

};

#endif // !__RESOURCE_BASE_H__