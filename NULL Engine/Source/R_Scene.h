#ifndef __R_SCENE_H__
#define __R_SCENE_H__

#include "Resource.h"

class ParsonNode;

class R_Scene : public Resource
{
public:
	R_Scene();
	~R_Scene();

	bool CleanUp();

	bool SaveMeta(ParsonNode& meta_root) const override;
	bool LoadMeta(const ParsonNode& meta_root) override;

private:
	// NO IMPORT SETTINGS?
};

#endif // !__R_SCENE_H__