#ifndef __R_ANIMATION_H__
#define __R_ANIMATION_H__

#include <vector>
#include <map>

#include "Resource.h"
#include "AnimationSettings.h"

class ParsonNode;
struct Channel;

class R_Animation : public Resource
{
public:
	R_Animation();
	~R_Animation();

	bool CleanUp();

	bool SaveMeta(ParsonNode& metaRoot) const override;
	bool LoadMeta(const ParsonNode& metaRoot) override;

public:
	const char* GetName				() const;
	double		GetDuration			() const;
	double		GetTicksPerSecond	() const;

	void		SetName				(const char* name);
	void		SetDuration			(const double& duration);
	void		SetTicksPerSecond	(const double& ticksPerSecond);

public:
	std::vector<Channel>	channels;

private:
	std::string				name;
	double					duration;
	double					ticksPerSecond;
	
	AnimationSettings animationSettings;
};

#endif // !__R_ANIMATION_H__