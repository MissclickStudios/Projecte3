#ifndef __R_2DANIMATION_H__
#define __R_2DANIMATION_H__

#include <vector>
#include <map>

#include "Resource.h"

class ParsonNode;
struct Channel;

class R_2DAnimation : public Resource
{
public:
	R_2DAnimation();
	~R_2DAnimation();

	bool CleanUp();

	bool SaveMeta(ParsonNode& metaRoot) const override;
	bool LoadMeta(const ParsonNode& metaRoot) override;

	//static inline ResourceType GetType() { return ResourceType::2D_ANIMATION; }

public:

	const char* GetName() const;
	double		GetDuration() const;

	void		SetName(const char* name);
	void		SetDuration(const double& duration);

private:
	std::string				name;
	double					duration;

};

#endif // !__R_2DANIMATION_H__