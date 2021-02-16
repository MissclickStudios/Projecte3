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

	bool SaveMeta(ParsonNode& meta_root) const override;
	bool LoadMeta(const ParsonNode& meta_root) override;

public:
	const char* GetName				() const;
	double		GetDuration			() const;
	double		GetTicksPerSecond	() const;

	void		SetName				(const char* name);
	void		SetDuration			(const double& duration);
	void		SetTicksPerSecond	(const double& ticks_per_second);

public:
	std::vector<Channel>	channels;

private:
	std::string				name;
	double					duration;
	double					ticks_per_second;
	
	AnimationSettings animation_settings;
};

#endif // !__R_ANIMATION_H__