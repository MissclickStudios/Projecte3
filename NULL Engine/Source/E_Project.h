#ifndef __E_PROJECT_H__
#define __E_PROJECT_H__

#include <string>

#include "PathNode.h"
#include "Icons.h"

#include "EditorPanel.h"

class R_Texture;
enum class ResourceType;

typedef unsigned int		uint;
typedef unsigned __int32	uint32;

struct AssetDisplay
{
	//AssetDisplay() : path(nullptr), file(nullptr), type((ResourceType)8), assetTexture(nullptr) {}
	AssetDisplay(const char* path, std::string file, ResourceType type, R_Texture* assetTexture) : path(path), file(file), type(type), assetTexture(assetTexture) {}
	
	const char*		path;
	std::string		file;
	ResourceType	type;
	R_Texture*		assetTexture;																	// Quick path to check if it works.
};

class E_Project : public EditorPanel
{
public:
	E_Project();
	~E_Project();

	bool Draw		(ImGuiIO& io) override;
	bool CleanUp	() override;

private:
	void			CheckFlags						();
	void			OnResize						();
	void			GenerateDockspace				(ImGuiIO& io) const;										// 

private:
	void			DrawMenuBar							();															// 
	void			DrawAssetsTree						();															// 
	void			DrawFolderExplorer					();															// 

	void			DrawDirectoriesTree					(const char* rootNode, const char* extensionToFilter);
	void			DrawDirectoriesTree					(const PathNode& rootNode);

	void			DrawResourceIcons					();

private:
	void			DrawGoToPreviousDirectoryButton		();
	void			AssetDragAndDropEvent				(const char* assetsPath, ImTextureID textureID);
	
	ImTextureID		GetIconTexID						(const AssetDisplay& assetDisplay) const;
	uint			GetOGLTextureID						(R_Texture* assetTexture) const;

	std::string		GetDisplayString					(std::string originalString, uint maxLenght) const;
	void			ClearAssetsToDisplay				();

	bool			DirectoryToDisplayIsRootDirectory	();

private:																									// --- ENGINE DIRECTORIES VARS
	PathNode				rootDirectory;

	char*					directoryToDisplay;
	PathNode				displayDirectory;

	std::vector<AssetDisplay>	assetsToDisplay;

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
};

#endif // !__E_PROJECT_H__