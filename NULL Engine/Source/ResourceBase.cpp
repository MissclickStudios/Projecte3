#include "Resource.h"						// TMP until Resource inherits from ResourceBase.

#include "ResourceBase.h"

ResourceBase::ResourceBase(std::string libraryPath, ResourceType type) : 
libraryPath	(libraryPath), 
type		(type)
{

}

ResourceBase::~ResourceBase()
{

}

bool ResourceBase::CleanUp()
{
	return true;
}
