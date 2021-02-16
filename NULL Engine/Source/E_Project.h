#ifndef __E_PROJECT_H__
#define __E_PROJECT_H__

#include "PathNode.h"
#include "Icons.h"

#include "EditorPanel.h"

class Resource;

typedef unsigned __int32 uint32;

class E_Project : public EditorPanel
{
public:
	E_Project();
	~E_Project();

	bool Draw		(ImGuiIO& io) override;
	bool CleanUp	() override;

public:
	Resource*		GetDraggedResource			();

private:
	void			CheckFlags					();
	void			OnResize					();
	void			GenerateDockspace			(ImGuiIO& io) const;										// 

private:
	void			DrawMenuBar					() const;													// 
	void			DrawAssetsTree				();															// 
	void			DrawFolderExplorer			();															// 

	void			DrawDirectoriesTree			(const char* root_node, const char* extension_to_filter);
	void			DrawDirectoriesTree			(const PathNode& root_node);

	void			DrawResourceIcons			();

private:
	void			GoToPreviousDirectoryButton	();
	void			ResourceDragAndDropEvent	(Resource* resource, ImTextureID texture_id);
	
	ImTextureID		GetIconTexID				(Resource* resource) const;
	std::string		GetDisplayString			(std::string original_string, uint max_lenght) const;
	void			ClearResourcesToDisplay		();

private:																									// --- ENGINE DIRECTORIES VARS
	PathNode				rootDirectory;

	char*					directoryToDisplay;
	PathNode				displayDirectory;

	std::vector<Resource*>	resourcesToDisplay;

	bool					refreshRootDirectory;
	bool					refreshDirectoryToDisplay;
	bool					refreshWindowSize;

private:																									// --- ENGINE ICONS VARS
	Icons					engineIcons;
	bool					iconsAreLoaded;

	ImVec2					iconSize;
	ImVec2					iconOffset;
	ImVec2					textOffset;
	ImVec2					winSize;

	Resource*				draggedResource;
};

#endif // !__E_PROJECT_H__