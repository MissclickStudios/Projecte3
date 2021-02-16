#ifndef __RESOURCE_H__
#define __RESOURCE_H__

#include <vector>
#include <string>

#include "Log.h"
#include "ImportSettings.h"												// No need to centralize it, each resource will have their own settings.

class ParsonNode;

typedef unsigned int		uint;
typedef unsigned __int32	uint32;

enum class RESOURCE_TYPE
{
	MODEL,
	MESH,
	MATERIAL,
	TEXTURE,
	FOLDER,
	ANIMATION,
	SCENE,
	NONE
};

class Resource
{
public:
	Resource(RESOURCE_TYPE type);
	virtual ~Resource();

	virtual bool CleanUp();

	virtual bool SaveMeta(ParsonNode& meta_root) const;
	virtual bool LoadMeta(const ParsonNode& meta_root);

public:
	RESOURCE_TYPE		GetType					() const;
	const char*			GetTypeAsString			() const;
	
	uint32				GetUID					() const;
	void				ForceUID				(const uint32& UID);
	uint				GetReferences			() const;
	void				SetReferences			(const uint& references);

public:
	const char*			GetAssetsPath			() const;								// 
	const char*			GetAssetsFile			() const;								// 
	const char*			GetLibraryPath			() const;								// 
	const char*			GetLibraryFile			() const;								// 

	void				SetAssetsPath			(const char* assets_path);				// 
	void				SetAssetsFile			(const char* assets_file);				// 
	void				SetLibraryPath			(const char* library_path);				// 
	void				SetLibraryFile			(const char* library_file);				// 

	void				SetLibraryPathAndFile	();

	//ImporterSettings	GetImporterSettings	();
	//void				SetImporterSettings	(ImporterSettings importer_settings);

private:
	RESOURCE_TYPE		type;

	uint32				uid;															// UID for this Resource.
	uint				references;

	std::string			assets_path;													// Path of the file in the Assets directory. Will be used to avoid making duplicates.
	std::string			assets_file;													// File and extension string of the texture in the Assets directory.
	std::string			library_path;													// Path of the file in the Library directory. Will be used to avoid making duplicates.
	std::string			library_file;													// File and extension string of the texture in the Library directory.

	ImportSettings		import_settings;
};

#endif // !__RESOURCE_H__