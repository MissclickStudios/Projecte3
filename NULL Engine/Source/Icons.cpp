#include "VariableTypedefs.h"

#include "R_Texture.h"

#include "Icons.h"

Icons::Icons() : 
animation_icon	(nullptr),
file_icon		(nullptr),
folder_icon		(nullptr),
material_icon	(nullptr),
model_icon		(nullptr)
{

}

uint Icons::GetAnimationIconID() const
{
	return ((animation_icon != nullptr) ? animation_icon->GetTextureID() : 0);
}

uint Icons::GetFileIconID() const
{
	return ((file_icon != nullptr) ? file_icon->GetTextureID() : 0);
}

uint Icons::GetFolderIconID() const
{
	return ((folder_icon != nullptr) ? folder_icon->GetTextureID() : 0);
}

uint Icons::GetMaterialIconID() const
{
	return ((material_icon != nullptr) ? material_icon->GetTextureID() : 0);
}

uint Icons::GetModelIconID() const
{
	return ((model_icon != nullptr) ? model_icon->GetTextureID() : 0);
}