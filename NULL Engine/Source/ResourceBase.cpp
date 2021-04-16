#include "Resource.h"						// TMP until Resource inherits from ResourceBase.

#include "ResourceBase.h"

ResourceBase::ResourceBase(uint32 UID, const std::string& assetsPath, const std::string& libraryPath, const ResourceType& type) :
UID			(UID),
assetsPath	(assetsPath),
libraryPath	(libraryPath), 
type		(type)
{

}

ResourceBase::ResourceBase(const Resource* resource) :
UID			((resource != nullptr) ? resource->GetUID() : 0),
assetsPath	((resource != nullptr) ? resource->GetAssetsPath() : "[NONE]"),
libraryPath	((resource != nullptr) ? resource->GetLibraryPath() : "[NONE]"),
type		((resource != nullptr) ? resource->GetType() : ResourceType::NONE)
{

}

ResourceBase::~ResourceBase()
{

}

bool ResourceBase::CleanUp()
{
	return true;
}
