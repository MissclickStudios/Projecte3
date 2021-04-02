#ifndef __R_ANIMATION2D_H__
#define __R_ANIMATION2D_H__

#include "Resource.h"

class ParsonNode;
struct Channel;

class R_Animation2D : public Resource
{
public:
	R_Animation2D();
	~R_Animation2D();

	bool CleanUp();

	bool SaveMeta(ParsonNode& metaRoot) const override;
	bool LoadMeta(const ParsonNode& metaRoot) override;

private:
};

#endif // !__R_ANIMATION2D_H__