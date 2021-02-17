#include "VariableTypedefs.h"

#include "R_Texture.h"

#include "Icons.h"

Icons::Icons() : 
animationIcon	(nullptr),
fileIcon		(nullptr),
folderIcon		(nullptr),
materialIcon	(nullptr),
modelIcon		(nullptr)
{

}

uint Icons::GetAnimationIconID() const
{
	return ((animationIcon != nullptr) ? animationIcon->GetTextureID() : 0);
}

uint Icons::GetFileIconID() const
{
	return ((fileIcon != nullptr) ? fileIcon->GetTextureID() : 0);
}

uint Icons::GetFolderIconID() const
{
	return ((folderIcon != nullptr) ? folderIcon->GetTextureID() : 0);
}

uint Icons::GetMaterialIconID() const
{
	return ((materialIcon != nullptr) ? materialIcon->GetTextureID() : 0);
}

uint Icons::GetModelIconID() const
{
	return ((modelIcon != nullptr) ? modelIcon->GetTextureID() : 0);
}