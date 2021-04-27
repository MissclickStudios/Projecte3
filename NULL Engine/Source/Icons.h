#ifndef __ICONS_H__
#define __ICONS_H__

#include "Macros.h"

class R_Texture;

typedef unsigned int uint;

struct MISSCLICK_API Icons
{
	Icons();

	uint GetAnimationIconID	() const;
	uint GetFileIconID		() const;
	uint GetFolderIconID	() const;
	uint GetMaterialIconID	() const;
	uint GetModelIconID		() const;

	R_Texture* animationIcon;
	R_Texture* fileIcon;
	R_Texture* folderIcon;
	R_Texture* materialIcon;
	R_Texture* modelIcon;
};

#endif // !__ICONS_H__