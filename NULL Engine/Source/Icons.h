#ifndef __ICONS_H__
#define __ICONS_H__

class R_Texture;

typedef unsigned int uint;

struct Icons
{
	Icons();

	uint GetAnimationIconID	() const;
	uint GetFileIconID		() const;
	uint GetFolderIconID	() const;
	uint GetMaterialIconID	() const;
	uint GetModelIconID		() const;

	R_Texture* animation_icon;
	R_Texture* file_icon;
	R_Texture* folder_icon;
	R_Texture* material_icon;
	R_Texture* model_icon;
};

#endif // !__ICONS_H__