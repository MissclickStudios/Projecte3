#include "Resource.h"						// TMP until Resource inherits from ResourceBase.

#include "ResourceBase.h"

ResourceBase::ResourceBase(uint32 UID, const std::string& assetsPath, const std::string& assetsFile, const std::string& libraryPath, const std::string& libraryFile, const ResourceType& type) :
UID			(UID),
assetsPath	(assetsPath),
assetsFile	(assetsFile),
libraryPath	(libraryPath), 
libraryFile	(libraryFile),
type		(type)
{

}

ResourceBase::ResourceBase(const Resource* resource) :
UID			((resource != nullptr) ? resource->GetUID() : 0),
assetsPath	((resource != nullptr) ? resource->GetAssetsPath() : "[NONE]"),
assetsFile	((resource != nullptr) ? resource->GetAssetsFile() : "[NONE]"),
libraryPath	((resource != nullptr) ? resource->GetLibraryPath() : "[NONE]"),
libraryFile ((resource != nullptr) ? resource->GetLibraryFile() : "[NONE]"),
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
