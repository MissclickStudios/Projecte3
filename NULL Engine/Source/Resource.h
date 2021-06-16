#ifndef __RESOURCE_H__
#define __RESOURCE_H__

#include <vector>
#include <string>

#include "Log.h"
#include "ImportSettings.h"												// No need to centralize it, each resource will have their own settings.
#include "Macros.h"

class ParsonNode;

typedef unsigned int		uint;
typedef unsigned __int32	uint32;

enum class ResourceType													// IMPORTANT: Do not change the order of the Types! This will create a conflict with the saved files.
{
	MODEL,
	MESH,
	MATERIAL,
	TEXTURE,
	FOLDER,
	ANIMATION,
	SCENE,
	SHADER,
	PARTICLE_SYSTEM,
	PREFAB,
	SCRIPT,
	NAVMESH,
	NONE
};

class MISSCLICK_API Resource
{
public:
	Resource(ResourceType type);
	virtual ~Resource();

	virtual bool CleanUp();

	virtual bool SaveMeta(ParsonNode& metaRoot) const;
	virtual bool LoadMeta(const ParsonNode& metaRoot);

	virtual inline ResourceType GetType() { return type; }

public:
	ResourceType	GetType() const;																// 
	const char*		GetTypeAsString() const;														// 
	
	uint32			GetUID() const;																	// 
	void			ForceUID(const uint32& UID);													// 
	
	uint			GetReferences() const;															// 
	void			SetReferences(const uint& references);											// 
	void			ModifyReferences(int modification);												//

public:
	const char*		GetAssetsPath() const;															// 
	const char*		GetAssetsFile() const;															// 
	const char*		GetLibraryPath() const;															// 
	const char*		GetLibraryFile() const;															// 

	void			SetAssetsPath(const char* assetsPath);											// 
	void			SetAssetsFile(const char* assetsFile);											// 
	void			SetLibraryPath(const char* libraryPath);										// 
	void			SetLibraryFile(const char* libraryFile);										// 

	void			SetAssetsPathAndFile(const char* assetsPath, const char* assetsFile);			// 
	void			SetLibraryPathAndFile();														// 

public:
	bool			hasForcedUID;

private:
	ResourceType	type;

	uint32			uid;																			// UID for this Resource.
	uint			references;

	std::string		assetsPath;																		// Path of the file in the Assets directory. Will be used to avoid making duplicates.
	std::string		assetsFile;																		// File and extension string of the texture in the Assets directory.
	std::string		libraryPath;																	// Path of the file in the Library directory. Will be used to avoid making duplicates.
	std::string		libraryFile;																	// File and extension string of the texture in the Library directory.

	ImportSettings	importSettings;
};

#endif // !__RESOURCE_H__