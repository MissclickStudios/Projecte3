#ifndef __M_RESOURCE_MANAGER_H__
#define __M_RESOURCE_MANAGER_H__

#include <map>
#include <string>

#include "Module.h"

class ParsonNode;
class Resource;

enum class RESOURCE_TYPE;

typedef unsigned int		uint;
typedef unsigned __int32	uint32;
typedef unsigned __int64	uint64;

class M_ResourceManager : public Module
{
public:
	M_ResourceManager();
	~M_ResourceManager();

	bool			Init				(ParsonNode& configuration) override;
	bool			Start				() override;
	UPDATE_STATUS	PreUpdate			(float dt) override;
	UPDATE_STATUS	Update				(float dt) override;
	UPDATE_STATUS	PostUpdate			(float dt) override;
	bool			CleanUp				() override;

	bool			SaveConfiguration	(ParsonNode& configuration) const override;
	bool			LoadConfiguration	(ParsonNode& configuration) override;

public:																												// --- ASSETS MONITORING METHODS ---
	void			RefreshDirectoryFiles			(const char* directory);
	void			RefreshDirectory				(const char* directory, std::vector<std::string>& files_to_import, 
														std::vector<std::string>& files_to_update, std::vector<std::string>& files_to_delete);
	
	void			FindFilesToImport				(const std::vector<std::string>& asset_files, const std::vector<std::string>& meta_files, 
														std::map<std::string, std::string>& file_pairs, std::vector<std::string>& files_to_import);
	
	void			FindFilesToUpdate				(const std::map<std::string, std::string>& file_pairs, std::vector<std::string>& files_to_update);

	void			FindFilesToDelete				(const std::vector<std::string>& meta_files, const std::map<std::string, std::string>& file_pairs, 
														std::vector<std::string>& files_to_delete);

	void			LoadValidFilesIntoLibrary		(const std::map<std::string, std::string>& file_pairs);

	bool			DeleteFromAssets				(const char* assets_path);
	bool			DeleteFromLibrary				(const char* assets_path);

	bool			GetResourceUIDsFromMeta						(const char* assets_path, std::vector<uint32>& resource_UIDs);
	bool			GetLibraryFilePathsFromMeta					(const char* assets_path, std::vector<std::string>& file_paths);
	bool			GetLibraryDirectoryAndExtensionFromType		(const RESOURCE_TYPE& type, std::string& directory, std::string& extension);
	
	bool			LoadMetaLibraryPairsIntoLibrary				(const char* assets_path);
	bool			GetLibraryPairsFromMeta						(const char* assets_path, std::map<uint32, std::string>& pairs);

	uint64			GetAssetFileModTimeFromMeta		(const char* assets_path);

public:																												// --- IMPORT FILE METHODS ---
	uint32			ImportFile						(const char* assets_path);										// 
	uint32			ImportFromAssets				(const char* assets_path);										// 
	uint			SaveResourceToLibrary			(Resource* resource);											// 
	
	uint32			LoadFromLibrary					(const char* assets_path);										// 

	const char*		GetValidPath					(const char* assets_path);										// 
	RESOURCE_TYPE	GetTypeFromAssetsExtension		(const char* assets_path);										// 
	RESOURCE_TYPE	GetTypeFromLibraryExtension		(const char* library_path);										// 

	void			SetResourceAssetsPathAndFile	(const char* assets_path, Resource* resource);					// 
	void			SetResourceLibraryPathAndFile	(Resource* resource);											// 

public:																												// --- META FILE METHODS ---
	bool			SaveMetaFile					(Resource* resource) const;										//
	ParsonNode		LoadMetaFile					(const char* assets_path, char** buffer);						// Passing the buffer so it can be safely RELEASED after calling it.
	
	bool			HasMetaFile						(const char* assets_path);
	bool			MetaFileIsValid					(const char* assets_path);
	bool			MetaFileIsValid					(ParsonNode& meta_root);
	bool			ResourceHasMetaType				(Resource* resource) const;

	Resource*		GetResourceFromMetaFile			(const char* assets_path);

public:																												// --- RESOURCE METHODS ---
	Resource*		CreateResource					(RESOURCE_TYPE type, const char* assets_path = nullptr, const uint32& forced_UID = 0);	// 
	bool			DeleteResource					(const uint32& UID);																	//
	bool			DeleteResource					(Resource* resource_to_delete);															// FORCED DELETE
	void			GetResources					(std::map<uint32, Resource*>& resources) const;											// 
	
	Resource*		RequestResource					(const uint32& UID);																	// 
	bool			FreeResource					(const uint32& UID);																	// 
	
	Resource*		AllocateResource				(const uint32& UID, const char* assets_path = nullptr);									// 
	bool			DeallocateResource				(const uint32& UID);																	// 
	bool			DeallocateResource				(Resource* resource_to_deallocate);														// FORCED DEALLOCATE
	
private:
	std::map<uint32, Resource*>		resources;																		// Resources currently in memory.
	std::map<uint32, std::string>	library;																		// UID and Library Path string of all loaded resources.

	float							file_refresh_timer;																// 
	float							file_refresh_rate;																// 
};

#endif // !__M_RESOURCE_MANAGER_H__